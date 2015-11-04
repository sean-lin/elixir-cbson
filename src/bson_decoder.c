#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "cbson.h"

#define STACK_SIZE_INC 64

enum {
    st_doc=0,
    st_array,
    st_invalid,
} BsonState;

typedef struct {
    ErlNifEnv* env;
    cbson_st* atoms;

    ERL_NIF_TERM arg;

    size_t bytes_per_red;
    int return_lists;
    int return_trailer;
    int return_atom;
    ERL_NIF_TERM nil_term;

    unsigned char*  p;
    int             i;
    int             len;

    char* st_data;
    int* st_end;
    int st_size;
    int st_top;
} Decoder;
 
static inline
char dec_curr(Decoder* d) {
    return d->st_data[d->st_top-1];
}

static inline
int dec_curr_end(Decoder* d) {
    return d->st_end[d->st_top-1];
}

static inline
int dec_top(Decoder* d) {
    return d->st_top;
}

void dec_push(Decoder* d, char val, int len) {
    if(d->st_top >= d->st_size) {
        int new_sz = d->st_size + STACK_SIZE_INC;
        char* tmp_data = (char*) enif_alloc(new_sz * (sizeof(char) + sizeof(int)));
        int* tmp_len = (int*)(tmp_data + new_sz);
        memcpy(tmp_data, d->st_data, d->st_size * sizeof(char));
        memcpy(tmp_len, d->st_end, d->st_size * sizeof(int));
        enif_free(d->st_data);
        d->st_data = tmp_data;
        d->st_end = tmp_len; 
        d->st_size = new_sz;
        for(int i = d->st_top; i < d->st_size; i++) {
            d->st_data[i] = st_invalid;
            d->st_end[i] = 0;
        }
    }

    d->st_data[d->st_top] = val;
    d->st_end[d->st_top] = len;
    d->st_top++;
}

static
void dec_pop(Decoder* d, char val, int len) {
    assert(d->st_data[d->st_top-1] == val && "popped invalid state.");
    assert(d->st_end[d->st_top-1] == len && "popped invalid state.");
    d->st_data[d->st_top-1] = st_invalid;
    d->st_end[d->st_top-1] = 0;
    d->st_top--;
}

static
Decoder* dec_new(ErlNifEnv* env) {
    cbson_st* st = (cbson_st*)enif_priv_data(env);

    Decoder* d = (Decoder*)enif_alloc_resource(st->res_dec, sizeof(Decoder));

    if(d == NULL) {
        return NULL;
    }

    d->atoms = st;

    d->bytes_per_red = DEFAULT_BYTES_PER_REDUCTION;
    d->return_lists = 0;
    d->return_atom = 0;
    d->return_trailer = 0;
    d->nil_term = d->atoms->atom_nil;
    
    d->p = NULL;
    d->len = -1;
    d->i = -1;
  
    d->st_data = (char*) enif_alloc(STACK_SIZE_INC * (sizeof(char) + sizeof(int)));
    d->st_end = (int*)(d->st_data + STACK_SIZE_INC);
    d->st_size = STACK_SIZE_INC;
    d->st_top = 0;

    for(int i = 0; i < d->st_size; i++) {
        d->st_data[i] = st_invalid;
        d->st_end[i] = 0;
    }

    return d;
}

static
void dec_init(Decoder* d, ErlNifEnv* env, ERL_NIF_TERM arg, ErlNifBinary* bin) {
    d->env = env;
    d->arg = arg;

    d->p = (unsigned char*)bin->data;
    d->len = bin->size;
    d->i = 0;
}

void dec_destroy(ErlNifEnv* env, void* obj) {
    Decoder* d = (Decoder*)obj;

    if(d->st_data != NULL) {
        enif_free(d->st_data);
    }
}

static inline
ERL_NIF_TERM dec_error(Decoder* d, const char* err) {
    return make_error(d->atoms, d->env, err);
}

static 
int make_object(ErlNifEnv* env, ERL_NIF_TERM pairs, ERL_NIF_TERM* out, int ret_lists, int ret_atom) {
    ERL_NIF_TERM ret;
    ERL_NIF_TERM key;
    ERL_NIF_TERM val;

    if(!ret_lists) {
        ret = enif_make_new_map(env);
        while(enif_get_list_cell(env, pairs, &val, &pairs)) {
            if(!enif_get_list_cell(env, pairs, &key, &pairs)) {
                return 0;
            }
            if(ret_atom && !make_atom_by_binary(env, &key, &key)) {
                return 0;
            }
            if(!enif_make_map_put(env, ret, key, val, &ret)) {
                return 0;
            }
        }
    } else {
        ret = enif_make_list(env, 0);
        while(enif_get_list_cell(env, pairs, &val, &pairs)) {
            if(!enif_get_list_cell(env, pairs, &key, &pairs)) {
                return 0;
            }
            if(ret_atom && !make_atom_by_binary(env, &key, &key)) {
                return 0;
            }
            val = enif_make_tuple2(env, key, val);
            ret = enif_make_list_cell(env, val, ret);
        }
    }
    *out = ret;
    return 1;
}

static
int make_array(ErlNifEnv* env, ERL_NIF_TERM pairs, ERL_NIF_TERM* out) {
    int i = -1;
    int ki;
    ERL_NIF_TERM ret;
    ERL_NIF_TERM key;
    ERL_NIF_TERM val;
    
    ret = enif_make_list(env, 0);
    while(enif_get_list_cell(env, pairs, &val, &pairs)) {
        if(!enif_get_list_cell(env, pairs, &key, &pairs)) {
            return 0;
        }
        if(!enif_get_int(env, key, &ki)) {
            return 0;
        }
        if(i == -1) {
            i = ki;
        } else if(i != ki) {
            return 0;
        }
        i--;
        ret = enif_make_list_cell(env, val, ret);
    }
    if(i != -1) {
        return 0;
    }
    *out = ret;
    return 1;
}

static inline 
int read_bson_cstring(
        ErlNifEnv* env, Decoder* d, ERL_NIF_TERM *val, int max_len, int as_integer) {
    int i;
    unsigned char* ptr = d->p + d->i;
   
    for(i=0;;i++) {
        if(i>=max_len){
            return 0;
        }
        if(ptr[i] == '\0') {
           break; 
        }
    }
    if(as_integer) {
        int n = atoi((const char*)ptr);
        *val = enif_make_int(env, n);
    }else{
        *val = enif_make_sub_binary(env, d->arg, d->i, i);
    }
    d->i += i + 1;
    return 1;
}

static inline 
ERL_NIF_TERM read_bson_objectid(ErlNifEnv* env, Decoder* d) {
    ERL_NIF_TERM ret = enif_make_new_map(env);
    enif_make_map_put(env, ret, d->atoms->atom_struct, d->atoms->atom_objectid, &ret);

    enif_make_map_put(
            env, ret, 
            d->atoms->atom_objectid_value, enif_make_sub_binary(env, d->arg, d->i, 12),
            &ret);
    return ret;
}

static inline 
ERL_NIF_TERM read_bson_binary(ErlNifEnv* env, Decoder* d, uint8_t subtype, int32_t len) {
    ERL_NIF_TERM ret = enif_make_new_map(env);
    enif_make_map_put(env, ret, d->atoms->atom_struct, d->atoms->atom_bin, &ret);

    enif_make_map_put(
            env, ret, 
            d->atoms->atom_bin_value, enif_make_sub_binary(env, d->arg, d->i, len),
            &ret);
   
    enif_make_map_put(
            env, ret,
            d->atoms->atom_bin_subtype, enif_make_int(env, subtype),
            &ret);

    enif_make_map_put(
            env, ret, 
            d->atoms->atom_bin_value, enif_make_sub_binary(env, d->arg, d->i, len),
            &ret);
    return ret;
}

static inline 
ERL_NIF_TERM read_bson_utc(ErlNifEnv* env, Decoder* d, int64_t timestamp) {
    ERL_NIF_TERM ret = enif_make_new_map(env);
    enif_make_map_put(env, ret, d->atoms->atom_struct, d->atoms->atom_utc, &ret);
    enif_make_map_put(
            env, ret, 
            d->atoms->atom_utc_value, enif_make_int64(env, timestamp), 
            &ret);
    return ret;
}

ERL_NIF_TERM decode_init(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    Decoder* d;
    
    cbson_st* st = (cbson_st*)enif_priv_data(env);
    ERL_NIF_TERM tmp_argv[5];
    ERL_NIF_TERM opts;
    ERL_NIF_TERM val;
    
    if(argc != 2) {
        return enif_make_badarg(env);
    }

    d = dec_new(env);
    if(d == NULL) {
        return make_error(st, env, "internal_error");
    }
    
    tmp_argv[0] = argv[0];
    tmp_argv[1] = enif_make_resource(env, d);
    tmp_argv[2] = st->atom_error;
    tmp_argv[3] = enif_make_list(env, 0);
    tmp_argv[4] = enif_make_list(env, 0);

    enif_release_resource(d);

    opts = argv[1];
    if(!enif_is_list(env, opts)) {
        return enif_make_badarg(env);
    }

    while(enif_get_list_cell(env, opts, &val, &opts)) {
        if(get_bytes_per_iter(env, val, &(d->bytes_per_red))) {
            continue;
        } else if(enif_compare(val, d->atoms->atom_return_lists) == 0) {
            d->return_lists = 1;
        } else if(enif_compare(val, d->atoms->atom_return_trailer) == 0) {
            d->return_trailer = 1;
        } else if(enif_compare(val, d->atoms->atom_return_atom) == 0) {
            d->return_atom = 1;
        } else if(enif_compare(val, d->atoms->atom_use_null) == 0) {
            d->nil_term = d->atoms->atom_null;
        } else if(get_nil_term(env, val, &(d->nil_term))) {
            continue;
        } else {
            return enif_make_badarg(env);
        }
    }

    ErlNifBinary bin;
    if(!enif_inspect_binary(env, argv[0], &bin)) {
        return enif_make_badarg(env);
    }
    dec_init(d, env, argv[0], &bin);
    if(d->len < 5){
        return make_error(st, env, "invalid_bson");
    }
    int32_t len = *(int32_t*)(d->p + d->i);
    d->i += 4;
    int32_t end = d->i + len - 5;

    if(end + 1 != d->len || d->p[end] != 0x0) {
        return make_error(st, env, "invalid_bson");
    }
    dec_push(d, st_doc, end);
    return decode_iter(env, 5, tmp_argv);
}

#define ASSERT_LEN(d,l,m) if((d)->i + (l) > st_end) {\
    ret = dec_error((d), (m));\
    goto done;\
    }\

#define ASSERT_STRING_LEN(d,l,m) if((d)->p[(d)->i + (l)] != 0x0) {\
    ret = dec_error((d), (m));\
    goto done;\
    }\

ERL_NIF_TERM decode_iter(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    Decoder* d;
    cbson_st* st = (cbson_st*)enif_priv_data(env);

    ErlNifBinary bin;

    if(argc != 5) {
        return enif_make_badarg(env);
    } else if(!enif_inspect_binary(env, argv[0], &bin)) {
        return enif_make_badarg(env);
    } else if(!enif_get_resource(env, argv[1], st->res_dec, (void**) &d)) {
        return enif_make_badarg(env);
    } else if(!enif_is_list(env, argv[3])) {
        return enif_make_badarg(env);
    } else if(!enif_is_list(env, argv[4])) {
        return enif_make_badarg(env);
    }
    if(bin.data != d->p) {
        // maybe wrong
        return enif_make_badarg(env);
    }

    int bytes_start = d->i;

    ERL_NIF_TERM objs = argv[3];
    ERL_NIF_TERM curr = argv[4];
    
    ERL_NIF_TERM val = argv[2];
    ERL_NIF_TERM ename;
    ERL_NIF_TERM ret;

    while(d->i < bin.size) {
    next:
        if(should_yield(env, &bytes_start, d->i, d->bytes_per_red)) {
            return enif_make_tuple5(env, st->atom_iter, argv[1], val, objs, curr);
        }
       
        char curr_stack = dec_curr(d);
        int st_end = dec_curr_end(d);

        uint8_t type = *(uint8_t*)(d->p + d->i);
        d->i++;
        if(type == 0x0 && st_end == d->i - 1) {
            if(curr_stack == st_doc) {
                if(!make_object(env, curr, &val, d->return_lists, d->return_atom)) {
                    ret = dec_error(d, "internal_error1");
                    goto done;
                }
            } else if(curr_stack == st_array) {
                if(!make_array(env, curr, &val)) {
                    ret = dec_error(d, "internal_error2");
                    goto done;
                }
            }
            dec_pop(d, curr_stack, st_end);
            if(dec_top(d) == 0) {
                goto decode_done;
            }else{
                if(!enif_get_list_cell(env, objs, &ename, &objs)) {
                    ret = dec_error(d, "internal_error3");
                    goto done;
                }
                if(!enif_get_list_cell(env, objs, &curr, &objs)) {
                    ret = dec_error(d, "internal_error4");
                    goto done;
                }
            }
        } else {
            if(!read_bson_cstring(env, d, &ename, st_end - d->i, curr_stack==st_array)) {
                ret = dec_error(d, "invalid_ename");
                goto done;
            }
            int32_t len;
            switch(type) {
                case BSON_DOUBLE:
                    ASSERT_LEN(d, 8, "invalid_double");
                    double n = *(double*)(d->p + d->i);
                    val = enif_make_double(env, n);
                    d->i += 8;
                    break;
                case BSON_STRING:
                    ASSERT_LEN(d, 4, "invalid_string");
                    len = *(int32_t*)(d->p + d->i);
                    d->i += 4;
                    ASSERT_LEN(d, len, "invalid_string");
                    ASSERT_STRING_LEN(d, len - 1, "invalid_string");
                    val = enif_make_sub_binary(env, d->arg, d->i, len - 1);
                    d->i += len;
                    break;
                case BSON_DOCUMENT:
                    ASSERT_LEN(d, 4, "invalid_doc");
                    len = *(int32_t*)(d->p + d->i);
                    len -= 5;
                    d->i += 4;
                    ASSERT_LEN(d, len + 1, "invalid_doc");
                    ASSERT_STRING_LEN(d, len, "invalid_doc");
                    dec_push(d, st_doc, d->i + len);
                    objs = enif_make_list_cell(env, curr, objs);
                    objs = enif_make_list_cell(env, ename, objs);
                    curr = enif_make_list(env, 0);
                    goto next;
                    break;
                case BSON_ARRAY:
                    ASSERT_LEN(d, 4, "invalid_doc");
                    len = *(int32_t*)(d->p + d->i);
                    len -= 5;
                    d->i += 4;
                    ASSERT_LEN(d, len + 1, "invalid_doc");
                    ASSERT_STRING_LEN(d, len, "invalid_doc");
                    dec_push(d, st_array, d->i + len);
                    objs = enif_make_list_cell(env, curr, objs);
                    objs = enif_make_list_cell(env, ename, objs);
                    curr = enif_make_list(env, 0);
                    goto next;
                    break;
                case BSON_BINARY:
                    ASSERT_LEN(d, 5, "invalid_binary");
                    len = *(int32_t*)(d->p + d->i);
                    uint8_t subtype = *(uint8_t*)(d->p + d->i + 4);
                    d->i += 5;
                    ASSERT_LEN(d, len, "invalid_binary");
                    val = read_bson_binary(env, d, subtype, len);
                    d->i += len;
                    break;
                case BSON_OBJECTID:
                    ASSERT_LEN(d, 12, "invalid_objectid");
                    val = read_bson_objectid(env, d);
                    d->i += 12;
                    break;
                case BSON_BOOL:
                    ASSERT_LEN(d, 1, "invalid_bool");
                    int8_t boolean = *(int8_t*)(d->p + d->i);
                    val = boolean?d->atoms->atom_true:d->atoms->atom_false;
                    d->i += 1;
                    break;
                case BSON_UTC:
                    ASSERT_LEN(d, 8, "invalid_utc");
                    int64_t ts = *(int64_t*)(d->p + d->i);
                    val = read_bson_utc(env, d, ts);
                    d->i += 8;
                    break;
                case BSON_NULL:
                    val = d->nil_term;
                    break;
                case BSON_REGEX:
                    break;
                case BSON_JS:
                    break;
                case BSON_JS_WS:
                    break;
                case BSON_INT32:
                    ASSERT_LEN(d, 4, "invalid_int32");
                    int int32 = *(int32_t*)(d->p + d->i);
                    val = enif_make_int(env, int32);
                    d->i += 4;    
                    break;
                case BSON_TIMESTAMP:
                    break;
                case BSON_INT64:
                    ASSERT_LEN(d, 8, "invalid_int64");
                    int64_t int64 = *(int64_t*)(d->p + d->i);
                    val = enif_make_int64(env, int64);
                    d->i += 8;    
                    break;
                case BSON_MIN:
                    val = d->atoms->atom_bson_min;
                    break;
                case BSON_MAX:
                    val = d->atoms->atom_bson_max;
                    break;
                default:
                    ret = dec_error(d, "invalid_type");
                    goto done;
                    break;
            }
        }
        curr = enif_make_list_cell(env, ename, curr);
        curr = enif_make_list_cell(env, val, curr);
    }

decode_done:
    if(d->i < bin.size && d->return_trailer) {
        ERL_NIF_TERM trailer = enif_make_sub_binary(env, argv[0], d->i, bin.size - d->i);
        val = enif_make_tuple3(env, d->atoms->atom_has_trailer, val, trailer);
    } else if(d->i < bin.size) {
        ret = dec_error(d, "invalid_trailing_data");
        goto done;
    }

    ret = val;
done:
    return ret;
}
