#include "cbson.h"


static int load(ErlNifEnv* env, void** priv, ERL_NIF_TERM info) {
    cbson_st* st = enif_alloc(sizeof(cbson_st));
    if(st == NULL) {
        return 1;
    }

#define MA(a,b) (st->a= make_atom(env, (b)))

    MA(atom_ok,             "ok");
    MA(atom_error,          "error");
    MA(atom_nil,            "nil");
    MA(atom_null,           "null");
    MA(atom_true,           "true");
    MA(atom_false,          "false");
    MA(atom_force_utf8,     "force_utf8");
    MA(atom_iter,           "iter");
    MA(atom_bytes_per_iter, "bytes_per_iter");
    MA(atom_return_lists,   "return_lists");
    MA(atom_return_trailer, "return_trailer");
    MA(atom_has_trailer,    "has_trailer");
    MA(atom_return_atom,    "return_atom");
    MA(atom_use_null,       "use_null");
    MA(atom_nil_term,       "nil_term");

    MA(atom_struct, "__struct__");
    MA(atom_objectid, ATOM_OBJECTID);
    MA(atom_objectid_value, ATOM_OBJECTID_VALUE);
    MA(atom_utc, ATOM_UTC);
    MA(atom_utc_value, ATOM_UTC_VALUE);
    MA(atom_regex, ATOM_REGEX);
    MA(atom_regex_pattern, ATOM_REGEX_PATTERN);
    MA(atom_regex_opts, ATOM_REGEX_OPTS);
    MA(atom_js, ATOM_JS);
    MA(atom_timestamp, ATOM_TIMESTAMP);
    MA(atom_timestamp_value, ATOM_TS_VALUE);
    MA(atom_bson_min, ATOM_BSON_MIN);
    MA(atom_bson_max, ATOM_BSON_MAX);
    MA(atom_bin, ATOM_BIN);
    MA(atom_bin_value, ATOM_BIN_VALUE);
    MA(atom_bin_subtype, ATOM_BIN_SUBTYPE);
    
    st->res_dec = enif_open_resource_type(
            env,
            NULL,
            "bson_decoder",
            dec_destroy,
            ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER,
            NULL);

    st->res_enc = enif_open_resource_type(
            env,
            NULL,
            "bson_encoder",
            enc_destroy,
            ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER,
            NULL);

    *priv = (void*)st;
    return 0;
}

static int reload(ErlNifEnv* env, void** priv, ERL_NIF_TERM info) {
    return 0;
}

static int upgrade(ErlNifEnv* env, void** priv, void** old_priv, ERL_NIF_TERM info) {
    return reload(env, priv, info);
}

static void unload(ErlNifEnv* env, void* priv) {
    enif_free(priv);
    return;
}

static ErlNifFunc funcs[] = {
    {"nif_decode_init", 2, decode_init},
    {"nif_decode_iter", 5, decode_iter},
    {"nif_encode_init", 2, encode_init},
    {"nif_encode_iter", 2, encode_iter}
};

ERL_NIF_INIT(Elixir.CBson, funcs, &load, &reload, &upgrade, &unload);
