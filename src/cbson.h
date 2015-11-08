#ifndef __CBSON__
#include "erl_nif.h"

#define DEFAULT_BYTES_PER_REDUCTION 20

#define BSON_DOUBLE             0x01
#define BSON_STRING             0x02
#define BSON_DOCUMENT           0x03
#define BSON_ARRAY              0x04
#define BSON_BINARY             0x05
// deprecated Undefined         0x06
#define BSON_OBJECTID           0x07
#define BSON_BOOL               0x08
#define BSON_UTC                0x09
#define BSON_NULL               0x0A
#define BSON_REGEX              0x0B
// deprecated DBPointer         0x0C
#define BSON_JS                 0x0D
// deprecated                   0x0E
#define BSON_JS_WS              0x0F
#define BSON_INT32              0x10
#define BSON_TIMESTAMP          0x11
#define BSON_INT64              0x12
#define BSON_MIN                0xFF
#define BSON_MAX                0x7F

#define BSON_BIN_BIN            0x00
#define BSON_BIN_FUNC           0x01
// deprecated binary            0x02
// deprecated UUID              0x03
#define BSON_BIN_UUID           0x04
#define BSON_BIN_MD5            0x05
#define BSON_BIN_USER           0x80

#ifndef ERICMJ_MONGODB

#define ATOM_OBJECTID           "Elixir.Bson.ObjectId"
#define ATOM_OBJECTID_VALUE     "oid"
#define ATOM_UTC                "Elixir.Bson.UTC"
#define ATOM_UTC_VALUE          "ms"
#define ATOM_REGEX              "Elixir.Bson.Regex"
#define ATOM_REGEX_PATTERN      "pattern"
#define ATOM_REGEX_OPTS         "opts"
#define ATOM_JS                 "Elixir.Bson.JS"
#define ATOM_TIMESTAMP          "Elixir.Bson.Timestamp"
#define ATOM_TS_VALUE           "ts"
#define ATOM_BSON_MIN           "min_key"
#define ATOM_BSON_MAX           "max_key"
#define ATOM_BIN                "Elixir.Bson.Bin"
#define ATOM_BIN_VALUE          "bin"
#define ATOM_BIN_SUBTYPE        "subtype"
#else

#endif // ERICMJ_MONGODB

typedef struct {
    ERL_NIF_TERM atom_ok;
    ERL_NIF_TERM atom_error;
    ERL_NIF_TERM atom_nil;
    ERL_NIF_TERM atom_null;
    ERL_NIF_TERM atom_true;
    ERL_NIF_TERM atom_false;
    ERL_NIF_TERM atom_force_utf8;
    ERL_NIF_TERM atom_iter;
    ERL_NIF_TERM atom_bytes_per_iter;
    ERL_NIF_TERM atom_return_lists;
    ERL_NIF_TERM atom_return_trailer;
    ERL_NIF_TERM atom_has_trailer;
    ERL_NIF_TERM atom_use_null;
    ERL_NIF_TERM atom_nil_term;
    ERL_NIF_TERM atom_return_atom;

    ERL_NIF_TERM atom_struct;
    ERL_NIF_TERM atom_objectid;
    ERL_NIF_TERM atom_objectid_value;
    ERL_NIF_TERM atom_utc;
    ERL_NIF_TERM atom_utc_value;
    ERL_NIF_TERM atom_regex;
    ERL_NIF_TERM atom_regex_pattern;
    ERL_NIF_TERM atom_regex_opts;
    ERL_NIF_TERM atom_js;
    ERL_NIF_TERM atom_timestamp;
    ERL_NIF_TERM atom_timestamp_value;
    ERL_NIF_TERM atom_bson_min;
    ERL_NIF_TERM atom_bson_max;
    ERL_NIF_TERM atom_bin;
    ERL_NIF_TERM atom_bin_value;
    ERL_NIF_TERM atom_bin_subtype;

    ErlNifResourceType* res_dec;
    ErlNifResourceType* res_enc;
} cbson_st;

ERL_NIF_TERM make_atom(ErlNifEnv* env, const char* name);
int make_atom_by_binary(ErlNifEnv* env, ERL_NIF_TERM* out, ERL_NIF_TERM* bin);
ERL_NIF_TERM make_error(cbson_st* st, ErlNifEnv* env, const char* error);
ERL_NIF_TERM make_obj_error(cbson_st* st, ErlNifEnv* env, const char* error, ERL_NIF_TERM obj);
int get_bytes_per_iter(ErlNifEnv* env, ERL_NIF_TERM val, size_t* bpi);
int get_nil_term(ErlNifEnv* env, ERL_NIF_TERM val, ERL_NIF_TERM *nil_term);
int should_yield(ErlNifEnv* env, int* start, int end, size_t bytes_per_red);

ERL_NIF_TERM encode_init(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM encode_iter(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
void enc_destroy(ErlNifEnv* env, void* obj);

ERL_NIF_TERM decode_init(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM decode_iter(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
void dec_destroy(ErlNifEnv* env, void* obj);

#endif // __CBSON__
