#include "cbson.h"
#include <string.h>

ERL_NIF_TERM make_atom(ErlNifEnv *env, const char *name)
{
    ERL_NIF_TERM ret;
    if (enif_make_existing_atom(env, name, &ret, ERL_NIF_LATIN1))
    {
        return ret;
    }
    return enif_make_atom(env, name);
}

int make_atom_by_binary(ErlNifEnv *env, ERL_NIF_TERM *in, ERL_NIF_TERM *out)
{
    ErlNifBinary bin;
    ERL_NIF_TERM ret;
    if (!enif_inspect_binary(env, *in, &bin))
    {
        return 0;
    }
    if (!enif_make_existing_atom_len(env, (const char *)bin.data, bin.size, out, ERL_NIF_LATIN1))
    {
        *out = *in;
    }
    return 1;
}

ERL_NIF_TERM make_error(cbson_st *st, ErlNifEnv *env, const char *error)
{
    return enif_make_tuple2(env, st->atom_error, make_atom(env, error));
}

ERL_NIF_TERM make_obj_error(cbson_st *st, ErlNifEnv *env, const char *error, ERL_NIF_TERM obj)
{
    return enif_make_tuple2(env, st->atom_error,
                            enif_make_tuple2(env, make_atom(env, error), obj));
}

int get_bytes_per_iter(ErlNifEnv *env, ERL_NIF_TERM val, size_t *bpi)
{
    cbson_st *st = (cbson_st *)enif_priv_data(env);
    const ERL_NIF_TERM *tuple;
    int arity;
    unsigned int bytes;

    if (!enif_get_tuple(env, val, &arity, &tuple))
    {
        return 0;
    }

    if (arity != 2)
    {
        return 0;
    }

    if (enif_compare(tuple[0], st->atom_bytes_per_iter) != 0)
    {
        return 0;
    }

    if (!enif_get_uint(env, tuple[1], &bytes))
    {
        return 0;
    }

    *bpi = (size_t)bytes;

    return 1;
}

int get_nil_term(ErlNifEnv *env, ERL_NIF_TERM val, ERL_NIF_TERM *nil_term)
{
    cbson_st *st = (cbson_st *)enif_priv_data(env);
    const ERL_NIF_TERM *tuple;
    int arity;

    if (!enif_get_tuple(env, val, &arity, &tuple))
    {
        return 0;
    }

    if (arity != 2)
    {
        return 0;
    }

    if (enif_compare(tuple[0], st->atom_nil_term) != 0)
    {
        return 0;
    }

    if (!enif_is_atom(env, tuple[1]))
    {
        return 0;
    }

    *nil_term = tuple[1];

    return 1;
}

int should_yield(ErlNifEnv *env, int *start, int end, size_t bytes_per_red)
{
    int used = end - *start;

    if (((used) / bytes_per_red) >= 20)
    {
        *start = end;
        return enif_consume_timeslice(env, 1);
    }

    return 0;
}

ERL_NIF_TERM objectid2bin(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    static const char *base16charidx = "0123456789abcdef";

    ErlNifBinary bin;
    ERL_NIF_TERM ret;
    if (!enif_inspect_binary(env, argv[0], &bin) || bin.size != 12)
    {
        return enif_make_badarg(env);
    }

    unsigned char *ret_bin = enif_make_new_binary(env, 24, &ret);
    for (int i = 0; i < 12; i++)
    {
        int j = i << 1;
        ret_bin[j] = base16charidx[bin.data[i] >> 4];
        ret_bin[j + 1] = base16charidx[bin.data[i] & 0xf];
    }
    return ret;
}

static inline unsigned char to_int(unsigned char i)
{
    return i >= 97 ? (i - 87) : (i - 48);
}

ERL_NIF_TERM bin2objectid(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifBinary bin;
    ERL_NIF_TERM ret;
    if (!enif_inspect_binary(env, argv[0], &bin) || bin.size != 24)
    {
        return enif_make_badarg(env);
    }

    unsigned char hi, lo;
    unsigned char *ret_bin = enif_make_new_binary(env, 12, &ret);
    for (int i = 0; i < 12; i++)
    {
        int j = i << 1;
        hi = bin.data[j];
        lo = bin.data[j + 1];
        if(hi < 48 || (hi > 57 && hi < 97) || hi > 102) {
            return enif_make_badarg(env);
        }
        if(lo < 48 || (lo > 57 && lo < 97) || lo > 102) {
            return enif_make_badarg(env);
        }
        ret_bin[i] = (to_int(hi) << 4) + to_int(lo);
    }
    return ret;
}

ERL_NIF_TERM b64encode(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    static const char *encoding = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    ErlNifBinary bin;
    ERL_NIF_TERM ret;

    if (!enif_inspect_binary(env, argv[0], &bin))
    {
        return enif_make_badarg(env);
    }

    const u_int8_t *text = bin.data;
    size_t sz = bin.size;

    int encode_sz = (sz + 2) / 3 * 4;

    unsigned char *buffer = enif_make_new_binary(env, encode_sz, &ret);

    int i, j;
    j = 0;
    for (i = 0; i < (int)sz - 2; i += 3)
    {
        u_int32_t v = text[i] << 16 | text[i + 1] << 8 | text[i + 2];
        buffer[j] = encoding[v >> 18];
        buffer[j + 1] = encoding[(v >> 12) & 0x3f];
        buffer[j + 2] = encoding[(v >> 6) & 0x3f];
        buffer[j + 3] = encoding[(v)&0x3f];
        j += 4;
    }
    int padding = sz - i;
    u_int32_t v;
    switch (padding)
    {
    case 1:
        v = text[i];
        buffer[j] = encoding[v >> 2];
        buffer[j + 1] = encoding[(v & 3) << 4];
        buffer[j + 2] = '=';
        buffer[j + 3] = '=';
        break;
    case 2:
        v = text[i] << 8 | text[i + 1];
        buffer[j] = encoding[v >> 10];
        buffer[j + 1] = encoding[(v >> 4) & 0x3f];
        buffer[j + 2] = encoding[(v & 0xf) << 2];
        buffer[j + 3] = '=';
        break;
    }
    return ret;
}

ERL_NIF_TERM split_by_char(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifBinary bin;
    ERL_NIF_TERM ret;
    int c, start;

    if(argc < 3) {
        return enif_make_badarg(env);
    }

    if(!enif_inspect_binary(env, argv[0], &bin)) {
        return enif_make_badarg(env);
    }

    if(!enif_get_int(env, argv[1], &c) || c > 255 || c < 0 ) {
        return enif_make_badarg(env);
    }
    
    if(!enif_get_int(env, argv[2], &start) || start >= bin.size) {
        return enif_make_badarg(env);
    }

    unsigned char* pos = (unsigned char*)memchr(bin.data + start, c, bin.size - start);
    if(pos == NULL) {
        return enif_make_badarg(env);
    }

    ERL_NIF_TERM begin = enif_make_sub_binary(env, argv[0], start, pos - bin.data - start);
    ERL_NIF_TERM rest = enif_make_sub_binary(
            env, argv[0], pos - bin.data + 1, bin.data + bin.size - pos - 1);

    return enif_make_tuple2(env, begin, rest);
}
