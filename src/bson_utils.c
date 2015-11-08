#include "cbson.h"

ERL_NIF_TERM make_atom(ErlNifEnv* env, const char* name) {
    ERL_NIF_TERM ret;
    if(enif_make_existing_atom(env, name, &ret, ERL_NIF_LATIN1)) {
        return ret;
    }
    return enif_make_atom(env, name);
}

int make_atom_by_binary(ErlNifEnv* env, ERL_NIF_TERM* in, ERL_NIF_TERM* out) {
    ErlNifBinary bin;
    ERL_NIF_TERM ret;
    if(!enif_inspect_binary(env, *in, &bin)) {
        return 0; 
    }
    if(!enif_make_existing_atom_len(env, (const char*)bin.data, bin.size, out, ERL_NIF_LATIN1)) {
        *out = *in;
    }
    return 1;
}

ERL_NIF_TERM make_error(cbson_st* st, ErlNifEnv* env, const char* error) {
    return enif_make_tuple2(env, st->atom_error, make_atom(env, error));
}

ERL_NIF_TERM make_obj_error(cbson_st* st, ErlNifEnv* env, const char* error, ERL_NIF_TERM obj) {
    return enif_make_tuple2(env, st->atom_error, 
            enif_make_tuple2(env, make_atom(env, error), obj));
}

int get_bytes_per_iter(ErlNifEnv* env, ERL_NIF_TERM val, size_t* bpi)
{
    cbson_st* st = (cbson_st*) enif_priv_data(env);
    const ERL_NIF_TERM* tuple;
    int arity;
    unsigned int bytes;

    if(!enif_get_tuple(env, val, &arity, &tuple)) {
        return 0;
    }

    if(arity != 2) {
        return 0;
    }

    if(enif_compare(tuple[0], st->atom_bytes_per_iter) != 0) {
        return 0;
    }

    if(!enif_get_uint(env, tuple[1], &bytes)) {
        return 0;
    }

    *bpi = (size_t) bytes;

    return 1;
}

int get_nil_term(ErlNifEnv* env, ERL_NIF_TERM val, ERL_NIF_TERM *nil_term) {
    cbson_st* st = (cbson_st*) enif_priv_data(env);
    const ERL_NIF_TERM* tuple;
    int arity;

    if(!enif_get_tuple(env, val, &arity, &tuple)) {
      return 0;
    }

    if(arity != 2) {
      return 0;
    }

    if(enif_compare(tuple[0], st->atom_nil_term) != 0) {
      return 0;
    }

    if(!enif_is_atom(env, tuple[1])) {
      return 0;
    }

    *nil_term = tuple[1];

    return 1;
}

int should_yield(ErlNifEnv* env, int* start, int end, size_t bytes_per_red)
{
    int used = end - *start;

    if(((used) / bytes_per_red) >= 20) {
        *start = end;
        return enif_consume_timeslice(env, 1);
    }

    return 0;
}
