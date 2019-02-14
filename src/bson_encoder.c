#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cbson.h"

#define BIN_ARRAY_SIZE 16
#define BIN_BLOCK_SIZE 2048

#define STACK_SIZE_INC 32

#define STACK_TYPE_UNDEFINED -3
#define STACK_TYPE_DOC_LIST -2
#define STACK_TYPE_DOC_MAP -1
#define STACK_TYPE_ARRAY 0

typedef struct { 
    int32_t status; // negative: doc, nonnegative: index of array
    int32_t* ptr; // The pointer of length;
    int32_t written;
    ErlNifMapIterator iter;
} Stack;

typedef struct {
    ErlNifEnv* env;
    cbson_st* atoms;

    size_t bytes_per_red;
    ERL_NIF_TERM nil_term;


    Stack* st_data;
    int st_top;         // 栈高
    int st_size;        // 栈大小

    ErlNifBinary* bin;
    int bin_top;
    int bin_size;
    int i;
} Encoder;

static Stack* enc_curr(Encoder*);

static
ErlNifBinary* enc_alloc_bin(Encoder* e) {
    if(e->bin_top >= e->bin_size) {
        int new_sz = e->bin_size + BIN_ARRAY_SIZE;
        ErlNifBinary* tmp_ptr = (ErlNifBinary*) enif_alloc(new_sz * sizeof(ErlNifBinary));
        memcpy(tmp_ptr, e->bin, e->bin_size * sizeof(ErlNifBinary));
        enif_free(e->bin);
        e->bin = tmp_ptr;
        e->bin_size = new_sz;
    }

    ErlNifBinary* bin = e->bin + e->bin_top;
    if(!enif_alloc_binary(BIN_BLOCK_SIZE, bin)){ 
        return NULL;
    }
    e->bin_top++;
    e->i = 0;
    return bin;
}

static inline
int enc_write_len(Encoder* e) {
    return e->i + e->bin_top?(e->bin_top*BIN_BLOCK_SIZE):0;
}

static inline
int enc_write_bin(Encoder* e, uint8_t* data, size_t len) {
    ErlNifBinary* bin = e->bin + e->bin_top - 1;
    enc_curr(e)->written += len;

    while(e->i + len > bin->size) {
        int rest = bin->size - e->i;
        if(rest > 0) {
            memcpy(bin->data + e->i, data, rest);
            len -= rest;
            data += rest;
        }
        e->i = bin->size;
        bin = enc_alloc_bin(e); 
        if(!bin) {
            return 0;
        }
    }

    memcpy(bin->data + e->i, data, len);
    e->i += len;
    return 1;
}

static 
unsigned char* enc_skip_len(Encoder* e, int32_t len) {
    ErlNifBinary* bin = e->bin + e->bin_top - 1;
    
    enc_curr(e)->written += len;

    if(e->i + len > bin->size) {
        if(e->i < bin->size) {
            enif_realloc_binary(bin, e->i);
        }
        bin = enc_alloc_bin(e); 
    }
    unsigned char* ptr = bin->data + e->i;
    e->i += len;
    return ptr;
}

static inline
void enc_write_uint8(Encoder* e, uint8_t d) {
    unsigned char* ptr = enc_skip_len(e, sizeof(uint8_t));
    *ptr = d;
}

static inline 
void enc_string(Encoder* e, unsigned char* data, int32_t size) {
    size += 1;
    enc_write_bin(e, (unsigned char*)&(size), sizeof(size));
    enc_write_bin(e, data, size - 1);
    enc_write_uint8(e, 0x0);
}

static inline
int enc_atom(Encoder* e, ERL_NIF_TERM val) {
    char atom[512];
    if(!enif_get_atom(e->env, val, atom, 512, ERL_NIF_LATIN1)) {
        return 0;
    }
    int32_t size = strlen(atom);
    enc_string(e, (unsigned char*)atom, size);
    return 1;
}

static 
int enc_ename(Encoder* e, ERL_NIF_TERM val) {
    char atom[512];
    ErlNifBinary bin;

    if(enif_is_binary(e->env, val)) {
        if(!enif_inspect_binary(e->env, val, &bin)) {
            return 0;
        }
        enc_write_bin(e, bin.data, bin.size);
        enc_write_uint8(e, 0x0);
    } else if(enif_is_atom(e->env, val)) {
        if(!enif_get_atom(e->env, val, atom, 512, ERL_NIF_LATIN1)) {
            return 0;
        }
        int32_t size = strlen(atom);
        enc_write_bin(e, (unsigned char*) atom, size);
        enc_write_uint8(e, 0x0);
    } else {
        return 0;
    }
    return 1;
}

static inline
void enc_idx_ename(Encoder* e, int32_t val) {
    char str[32];
    int len = snprintf(str, 32, "%d", val);
    enc_write_bin(e, (unsigned char*)str, len + 1);
}

static inline 
int enc_objectid(Encoder* e, ERL_NIF_TERM doc) {
    ERL_NIF_TERM oid;
    ErlNifBinary bin;
    if(enif_get_map_value(e->env, doc, e->atoms->atom_objectid_value, &oid)) {
        if(enif_inspect_binary(e->env, oid, &bin)) {
            if(bin.size == 12) {
                return enc_write_bin(e, bin.data, bin.size);
            }
        }
    }
    return 0;
}

static inline 
int enc_utc(Encoder* e, ERL_NIF_TERM doc) {
    ERL_NIF_TERM utc;
    ErlNifSInt64 ms;
    if(enif_get_map_value(e->env, doc, e->atoms->atom_utc_value, &utc)) {
        if(enif_get_int64(e->env, utc, &ms)) {
            int64_t ims = ms;
            return enc_write_bin(e, (unsigned char*)&ims, sizeof(ims));
        }
    }
    return 0;
}

static inline 
int enc_regex(Encoder* e, ERL_NIF_TERM doc) {
    ERL_NIF_TERM val;
    ErlNifBinary bin;

    if(enif_get_map_value(e->env, doc, e->atoms->atom_regex_pattern, &val) \
            && enif_inspect_binary(e->env, val, &bin)) {
        enc_write_bin(e, bin.data, bin.size);
        enc_write_uint8(e, 0x0);
        if(enif_get_map_value(e->env, doc, e->atoms->atom_regex_opts, &val)\
                && enif_inspect_binary(e->env, val, &bin)) {
            enc_write_bin(e, bin.data, bin.size);
            enc_write_uint8(e, 0x0);
            return 1;
        }
    }
    return 0;
}

static inline 
int enc_timestamp(Encoder* e, ERL_NIF_TERM doc) {
    ERL_NIF_TERM ts;
    ErlNifSInt64 ms;
    if(enif_get_map_value(e->env, doc, e->atoms->atom_timestamp_value, &ts)) {
        if(enif_get_int64(e->env, ts, &ms)) {
            int64_t ims = ms;
            return enc_write_bin(e, (unsigned char*)&ims, sizeof(ims));
        }
    }
    return 0;
}

static inline 
int enc_bin(Encoder* e, ERL_NIF_TERM doc) {
    ERL_NIF_TERM val;
    ErlNifBinary bin;
    int32_t s;
    
    if(enif_get_map_value(e->env, doc, e->atoms->atom_bin_subtype, &val)) {
        if(enif_get_int(e->env, val, &s)) {
            if(enif_get_map_value(e->env, doc, e->atoms->atom_bin_value, &val)) {
                if(enif_inspect_binary(e->env, val, &bin)) {
                    unsigned char subtype = s;
                    int32_t size = bin.size;
                    enc_write_bin(e, (unsigned char*)&size, sizeof(uint32_t));
                    enc_write_uint8(e, subtype);
                    enc_write_bin(e, bin.data, bin.size);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static inline 
int enc_stack_is_list(Stack* st) {
    return st->status >= 0;
}

static inline
Stack* enc_curr(Encoder* e) {
    return e->st_data + e->st_top - 1;
}

static
int32_t * enc_push(Encoder* e, int32_t status) {
    Stack* st;

    if(e->st_top >= e->st_size) {
        int new_sz = e->st_size + STACK_SIZE_INC;
        Stack* tmp_data = (Stack*) enif_alloc(new_sz * (sizeof(Stack)));
        memcpy(tmp_data, e->st_data, e->st_size * sizeof(Stack));
        enif_free(e->st_data);
        e->st_data = tmp_data;
        e->st_size = new_sz;
    }

    st = e->st_data + e->st_top;
    st->status = status;
    st->written = 0;
    e->st_top++;
    
    st->ptr = (int32_t*)enc_skip_len(e, sizeof(int32_t));
    return st->ptr;
}

static 
void enc_pop(Encoder* e) {
    Stack* st = e->st_data + e->st_top - 1;
    int32_t written = st->written;
    *(st->ptr) = written;

    st->status = STACK_TYPE_UNDEFINED; 
    st->ptr = 0; 
    st->written = 0;
    e->st_top--;
   
    if(e->st_top > 0) {
        st = e->st_data + e->st_top - 1;
        st->written += written; 
    }
}


static 
Encoder* enc_new(ErlNifEnv* env) {
    cbson_st* st = (cbson_st*)enif_priv_data(env);

    Encoder* e = (Encoder*)enif_alloc_resource(st->res_enc, sizeof(Encoder));

    if(e == NULL) {
        return NULL;
    }

    e->env = env;
    e->atoms = st;
    e->bytes_per_red = DEFAULT_BYTES_PER_REDUCTION;
    e->nil_term = e->atoms->atom_nil;
    
    e->i = 0;

    e->st_data = (Stack*)enif_alloc(STACK_SIZE_INC * sizeof(Stack));
    e->st_size = STACK_SIZE_INC;
    e->st_top = 0;
    
    e->bin = (ErlNifBinary*)enif_alloc(BIN_ARRAY_SIZE * sizeof(ErlNifBinary));
    e->bin_size = BIN_ARRAY_SIZE;
    e->bin_top = 0;
    enc_alloc_bin(e);

    return e;
}

void enc_destroy(ErlNifEnv* env, void* obj) {
    int i; 
    Encoder* e = (Encoder*)obj;

    if(e->st_data != NULL) {
        for(i = 0; i < e->st_top; i++) {
            Stack* st = e->st_data + i;
            if(!enc_stack_is_list(st) && st->status == STACK_TYPE_DOC_MAP) {
                enif_map_iterator_destroy(env, &(st->iter));
            }
        }
        enif_free(e->st_data);
    }
    if(e->bin != NULL) {
        for(i = 0; i < e->bin_top; i++) {
            enif_release_binary(e->bin + i);
        }
        enif_free(e->bin);
    }
}

static inline
ERL_NIF_TERM enc_error(Encoder* e, const char* err) {
    return make_error(e->atoms, e->env, err);
}

static inline
ERL_NIF_TERM enc_obj_error(Encoder* e, const char* err, ERL_NIF_TERM obj) {
    return make_obj_error(e->atoms, e->env, err, obj);
}

static inline
ERL_NIF_TERM make_last(ErlNifEnv* env, Encoder* e) {
    ErlNifBinary* bin = e->bin + e->bin_top - 1;
    if(e->i != bin->size){
        ERL_NIF_TERM tmp = enif_make_binary(env, bin);
        return enif_make_sub_binary(env, tmp, 0, e->i);
    }
    return enif_make_binary(env, bin);
}

static 
ERL_NIF_TERM make_result(ErlNifEnv* env, Encoder* e) {
    ERL_NIF_TERM last = make_last(env, e);
    if(e->bin_top == 1) {
        e->bin_top = 0;
        return last;
    }

    ErlNifBinary* bin;
    ERL_NIF_TERM ret = enif_make_list(env, 1, last);
    for(int i = e->bin_top - 2; i >= 0; i--) {
        bin = e->bin + i;
        ret = enif_make_list_cell(env, 
                enif_make_binary(env, bin),
                ret);
    }
    e->bin_top = 0;
    return ret;
}

ERL_NIF_TERM encode_init(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    cbson_st* st = (cbson_st*)enif_priv_data(env);
    ERL_NIF_TERM tmp_argv[3];
    ERL_NIF_TERM opts;
    ERL_NIF_TERM val;
    
    if(argc != 2) {
        return enif_make_badarg(env);
    }

    Encoder* e = enc_new(env);
    if(e == NULL) {
        return make_error(st, env, "internal_error");
    }
    
    tmp_argv[0] = enif_make_resource(env, e);
    tmp_argv[1] = enif_make_list(env, 1, argv[0]);

    enif_release_resource(e);

    opts = argv[1];
    if(!enif_is_list(env, opts)) {
        return enif_make_badarg(env);
    }

    while(enif_get_list_cell(env, opts, &val, &opts)) {
        if(get_bytes_per_iter(env, val, &(e->bytes_per_red))) {
            continue;
        } else if(enif_compare(val, e->atoms->atom_use_null) == 0) {
            e->nil_term = e->atoms->atom_null;
        } else if(get_nil_term(env, val, &(e->nil_term))) {
            continue;
        } else {
            return enif_make_badarg(env);
        }
    }

    if(enif_is_list(env, argv[0])) {
        enc_push(e, STACK_TYPE_DOC_LIST);
    }else if(enif_is_map(env, argv[0])) {
        ERL_NIF_TERM key;
        if(enif_get_map_value(env, argv[0], st->atom_struct, &key)) {
            return enif_make_badarg(env);
        }
        enc_push(e, STACK_TYPE_DOC_MAP);
        enif_map_iterator_create(
                env, argv[0], &(enc_curr(e)->iter), ERL_NIF_MAP_ITERATOR_FIRST);
    }
    
    return encode_iter(env, 2, tmp_argv);
}

ERL_NIF_TERM encode_iter(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]) {
    Encoder* e;
    cbson_st* st = (cbson_st*)enif_priv_data(env);

    if(argc != 2) {
        return enif_make_badarg(env);
    } else if(!enif_get_resource(env, argv[0], st->res_enc, (void**) &e)) {
        return enif_make_badarg(env);
    } else if(!(enif_is_list(env, argv[1]))) {
        return enif_make_badarg(env);
    }

    // 使用一个 erlang list来模拟栈
    ERL_NIF_TERM stack = argv[1];
    ERL_NIF_TERM curr, key, value, ret;

    int start = enc_write_len(e);

    const ERL_NIF_TERM* tuple;
    int arity;
    unsigned char* ptr;
    ErlNifBinary bin;
    ErlNifSInt64 lval;
    double dval;

    while(!enif_is_empty_list(env, stack)) {
        if(!enif_get_list_cell(env, stack, &curr, &stack)) {
            goto encode_done;
        }
        Stack* curr_stack = enc_curr(e);
next:
        if(should_yield(env, &start, enc_write_len(e), e->bytes_per_red)) {
            stack = enif_make_list_cell(env, curr, stack);
            return enif_make_tuple3(env, st->atom_iter, argv[0], stack);
        }
       
        // 准备好 key and value
        if(enc_stack_is_list(curr_stack)) {
            // 当前栈是 list
            if(!enif_get_list_cell(env, curr, &value, &curr)) {
                enc_write_uint8(e, 0x0);
                enc_pop(e);
                continue;
            }
            ptr = enc_skip_len(e, sizeof(unsigned char));
            enc_idx_ename(e, curr_stack->status);
            curr_stack->status += 1;
        } else {
            // 当前栈是 doc
            if(enif_is_list(env, curr)) {
                ERL_NIF_TERM item;
                if(!enif_get_list_cell(env, curr, &item, &curr)) {
                    enc_write_uint8(e, 0x0);
                    enc_pop(e);
                    continue;
                }
                if(!enif_get_tuple(env, item, &arity, &tuple)) {
                    ret = enc_obj_error(e, "invalid_object_member", item);
                    goto done;
                }
                if(arity != 2) {
                    ret = enc_obj_error(e,   "invalid_object_member", item);
                    goto done;
                }
                key = tuple[0];
                value = tuple[1];
            } else {
                if(curr_stack->status != STACK_TYPE_DOC_MAP) {
                    ret = enc_obj_error(e, "invalid_keywords", curr);
                    goto done;
                }
                if(!enif_map_iterator_get_pair(env, &(curr_stack->iter), &key, &value)) {
                    enif_map_iterator_destroy(env, &(curr_stack->iter));
                    enc_write_uint8(e, 0x0);
                    enc_pop(e);
                    continue;
                }
                enif_map_iterator_next(env, &(curr_stack->iter));
            }


            if(enif_is_atom(env, key) && enif_compare(key, st->atom_struct) == 0) {
                goto next;
            }
            
            ptr = enc_skip_len(e, sizeof(unsigned char));
            if(!enc_ename(e, key)) {
                ret = enc_obj_error(e, "invalid_string", key);
                goto done;
            }
        }

        // encode value
        if(enif_is_binary(env, value)) {
            *ptr = BSON_STRING;
            enif_inspect_binary(env, value, &bin);
            enc_string(e, bin.data, bin.size);
        } else if(enif_is_atom(env, value)) {
            if(enif_compare(value, e->nil_term) == 0) {
                *ptr = BSON_NULL;
            } else if(enif_compare(value, st->atom_true) == 0) {
                *ptr = BSON_BOOL;
                enc_write_uint8(e, 0x1);
            } else if(enif_compare(value, st->atom_false) == 0) {
                *ptr = BSON_BOOL;
                enc_write_uint8(e, 0x0);
            } else if(enif_compare(value, st->atom_nan) == 0) {
                *ptr = BSON_DOUBLE;
                enc_write_bin(e, (unsigned char*)nan1, 8);
            } else if(enif_compare(value, st->atom_inf) == 0) {
                *ptr = BSON_DOUBLE;
                enc_write_bin(e, (unsigned char*)inf, 8);
            } else if(enif_compare(value, st->atom_ninf) == 0) {
                *ptr = BSON_DOUBLE;
                enc_write_bin(e, (unsigned char*)ninf, 8);
            } else if(enif_compare(value, st->atom_bson_min) == 0) {
                *ptr = BSON_MIN;
            } else if(enif_compare(value, st->atom_bson_max) == 0) {
                *ptr = BSON_MAX;
            } else {
                *ptr = BSON_STRING;
                enc_atom(e, value);
            }
        } else if(enif_get_int64(env, value, &lval)) {
            if(lval >= INT32_MIN && lval <= INT32_MAX) {
                *ptr = BSON_INT32;
                int32_t d = lval;
                enc_write_bin(e, (unsigned char*)&d, sizeof(d));
            } else {
                *ptr = BSON_INT64;
                enc_write_bin(e, (unsigned char*)&lval, sizeof(lval));
            }
        } else if(enif_get_double(env, value, &dval)) {
            *ptr = BSON_DOUBLE;
            enc_write_bin(e, (unsigned char*)&dval, sizeof(dval));
        } else if(enif_is_map(env, value)) {
            ERL_NIF_TERM key;
            if(enif_get_map_value(env, value, st->atom_struct, &key)){
                // struct
                if(enif_compare(key, st->atom_objectid) == 0) {
                    *ptr = BSON_OBJECTID;
                    if(!enc_objectid(e, value)) {
                        ret = enc_obj_error(e, "error_objid", value);
                        goto done;
                    }
                } else if(enif_compare(key, st->atom_utc) == 0) {
                    *ptr = BSON_UTC;
                    if(!enc_utc(e, value)) {
                        ret = enc_obj_error(e, "error_utc", value);
                        goto done;
                    }
                } else if(enif_compare(key, st->atom_bin) == 0) {
                    *ptr = BSON_BINARY;
                    if(!enc_bin(e, value)) {
                        ret = enc_obj_error(e, "error_bin", value);
                        goto done;
                    }
                } else if(enif_compare(key, st->atom_regex) == 0) {
                    *ptr = BSON_REGEX;
                    if(!enc_regex(e, value)) {
                        ret = enc_obj_error(e, "error_regex", value);
                        goto done;
                    }
                } else if(enif_compare(key, st->atom_timestamp) == 0) {
                    *ptr = BSON_TIMESTAMP;
                    if(!enc_timestamp(e, value)) {
                        ret = enc_obj_error(e, "error_timestamp", value);
                        goto done;
                    }
                } else {
                    goto encode_map_doc;
                }
            }else{
                // doc
encode_map_doc:
                *ptr = BSON_DOCUMENT;
                enc_push(e, STACK_TYPE_DOC_MAP);
                enif_map_iterator_create(
                        env, value, &(enc_curr(e)->iter), ERL_NIF_MAP_ITERATOR_FIRST);
                stack = enif_make_list_cell(env, curr, stack);
                stack = enif_make_list_cell(env, value, stack);
                continue;
            }
        } else if(enif_is_list(env, value)) {
            if(enif_is_empty_list(env, value)) {
                *ptr = BSON_ARRAY;
                enc_write_bin(e, (uint8_t*)"\5\0\0\0\0", 5);
            } else {
                stack = enif_make_list_cell(env, curr, stack);
                
                // test
                ERL_NIF_TERM test;
                enif_get_list_cell(env, value, &test, &curr);
                if(enif_is_tuple(env, test)) {
                    *ptr = BSON_DOCUMENT;
                    enc_push(e, STACK_TYPE_DOC_LIST);
                } else {
                    *ptr = BSON_ARRAY;
                    enc_push(e, STACK_TYPE_ARRAY);
                }
                stack = enif_make_list_cell(env, value, stack);
                continue;
            }
        } else {
            ret = enc_obj_error(e, "error_term", value);
            goto done;
        }
        goto next;
    }
encode_done:
    ret = make_result(env, e);

done:
    return ret;
}

