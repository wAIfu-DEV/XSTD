#pragma once

#include "xstd_core.h"
#include "xstd_error.h"

typedef union {
    Bool isErr;
    Error err;
} Res;

#define result_define(T, Alias) \
    typedef struct {             \
        Bool isErr;              \
        Error err;               \
        T value;                 \
    } Res##Alias

#define result_type(T) Res##T

#define result_err(ResName, Err) \
    (Res##ResName){               \
        .isErr = 1,               \
        .err = Err,               \
    }

#define result_ok(ResName, Val) \
    (Res##ResName){              \
        .isErr = 0,              \
        .value = Val,            \
    }


result_define(i64, I64);
result_define(u64, U64);
result_define(f64, F64);
result_define(i8, Byte);
result_define(String, Str);
result_define(OwnedStr, OwnedStr);
result_define(ConstStr, ConstStr);
result_define(Buffer, Buffer);
result_define(HeapBuff, OwnedBuff);
result_define(ConstBuff, ConstBuff);
