#pragma once

#include "xstd_core.h"
#include "xstd_errcode.h"
#include "xstd_error.h"
#include "xstd_result.h"

typedef struct _result_u8
{
    u8 value;
    Error error;
} ResultU8;

typedef struct _result_i8
{
    i8 value;
    Error error;
} ResultI8;

typedef struct _result_u16
{
    u16 value;
    Error error;
} ResultU16;

typedef struct _result_i16
{
    i16 value;
    Error error;
} ResultI16;

typedef struct _result_u32
{
    u32 value;
    Error error;
} ResultU32;

typedef struct _result_i32
{
    i32 value;
    Error error;
} ResultI32;

typedef struct _result_f32
{
    f32 value;
    Error error;
} ResultF32;

#define _addT(T)                            \
    static inline T math_##T##_add(T a, T b) \
    {                                        \
        return a + b;                        \
    }

#define _addNoOfwT(T, ResT, LT)                                                                                  \
    static inline ResT math_##T##_add_nooverflow(T a, T b)                                                        \
    {                                                                                                             \
        LT res = (LT)a + (LT)b;                                                                                   \
                                                                                                                  \
        if (res != (T)res)                                                                                        \
            return (ResT){                                                                                        \
                .value = 0,                                                                                       \
                .error = X_ERR_EXT("math", "math_T_add_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),      \
            };                                                                                                    \
                                                                                                                  \
        return (ResT){                                                                                            \
            .value = (T)res,                                                                                      \
            .error = X_ERR_OK,                                                                                    \
        };                                                                                                        \
    }

#define _substractT(T)                            \
    static inline T math_##T##_substract(T a, T b) \
    {                                              \
        return a - b;                              \
    }

#define _substractNoOfwT(T, ResT, LT)                                                                                  \
    static inline ResT math_##T##_substract_nooverflow(T a, T b)                                                        \
    {                                                                                                                   \
        LT res = (LT)a - (LT)b;                                                                                         \
                                                                                                                        \
        if (res != (T)res)                                                                                              \
            return (ResT){                                                                                              \
                .value = 0,                                                                                             \
                .error = X_ERR_EXT("math", "math_T_substract_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),      \
            };                                                                                                          \
                                                                                                                        \
        return (ResT){                                                                                                  \
            .value = (T)res,                                                                                            \
            .error = X_ERR_OK,                                                                                          \
        };                                                                                                              \
    }

#define _multiplyT(T)                                 \
    static inline T math_##T##_multiply(T x, T mulBy) \
    {                                                 \
        return x * mulBy;                             \
    }

#define _multiplyNoOfwT(T, ResT, LT)                                                                                   \
    static inline ResT math_##T##_multiply_nooverflow(T a, T b)                                                        \
    {                                                                                                                  \
        LT res = (LT)a * (LT)b;                                                                                        \
                                                                                                                       \
        if (res != (T)res)                                                                                             \
            return (ResT){                                                                                             \
                .value = 0,                                                                                            \
                .error = X_ERR_EXT("math", "math_T_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),      \
            };                                                                                                         \
                                                                                                                       \
        return (ResT){                                                                                                 \
            .value = (T)res,                                                                                           \
            .error = X_ERR_OK,                                                                                         \
        };                                                                                                             \
    }

#define _divideT(T, ResT)                                                                                 \
    static inline ResT math_##T##_divide(T x, T divBy)                                                    \
    {                                                                                                     \
        if ((T)divBy == (T)0)                                                                             \
            return (ResT){                                                                                \
                .value = (T)0,                                                                            \
                .error = X_ERR_EXT("math", "math_T_divide", ERR_INVALID_PARAMETER, "division by 0"),      \
            };                                                                                            \
                                                                                                          \
        return (ResT){                                                                                    \
            .value = (T)((T)x / (T)divBy),                                                                \
            .error = X_ERR_OK,                                                                            \
        };                                                                                                \
    }

#define _divideNoOfwT(T, ResT, LT)                                                                                   \
    static inline ResT math_##T##_divide_nooverflow(T x, T divBy)                                                    \
    {                                                                                                                \
        if (divBy == 0)                                                                                              \
            return (ResT){                                                                                           \
                .value = 0,                                                                                          \
                .error = X_ERR_EXT("math", "math_T_divide_nooverflow", ERR_INVALID_PARAMETER, "division by 0"),      \
            };                                                                                                       \
                                                                                                                     \
        LT res = (LT)x / (LT)divBy;                                                                                  \
                                                                                                                     \
        if (res != (T)res)                                                                                           \
            return (ResT){                                                                                           \
                .value = 0,                                                                                          \
                .error = X_ERR_EXT("math", "math_T_divide_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),      \
            };                                                                                                       \
                                                                                                                     \
        return (ResT){                                                                                               \
            .value = (T)res,                                                                                         \
            .error = X_ERR_OK,                                                                                       \
        };                                                                                                           \
    }

#define _IabsT(T)                         \
    static inline T math_##T##_abs(T x)   \
    {                                     \
        return (T)((x >= (T)0) ? x : -x); \
    }

#define _FroundT(T, LT)                   \
    static inline T math_##T##_round(T x) \
    {                                     \
        if (x >= 0.0)                     \
            return (T)(LT)(x + 0.5);      \
        else                              \
            return (T)(LT)(x - 0.4);      \
    }

#define _UpowT(T)                       \
    static inline T math_##T##_power(T x, T exponent) \
    {                                   \
        T r = 1;                        \
        T b = (exponent);               \
        for (T i = 0; i < b; ++i)       \
        {                               \
            r *= (x);                   \
        }                               \
        return r;                       \
    }

#define _UpowNoOfwT(T, ResT, LT)                                                                                       \
    static inline ResT math_##T##_power_nooverflow(T x, T exponent)                                                                   \
    {                                                                                                                   \
        LT r = 1;                                                                                                       \
        LT b = (exponent);                                                                                              \
        for (LT i = 0; i < b; ++i)                                                                                      \
        {                                                                                                               \
            r *= (x);                                                                                                   \
            if (r != (T)r)                                                                                              \
                return (ResT){                                                                                          \
                    .value = 0,                                                                                         \
                    .error = X_ERR_EXT("math", "math_T_power_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"), \
                };                                                                                                      \
        }                                                                                                               \
        return (ResT){                                                                                                  \
            .value = (T)r,                                                                                              \
            .error = X_ERR_OK,                                                                                          \
        };                                                                                                              \
    }

#define _IpowT(T)                             \
    static inline T math_##T##_power(T x, T exponent)        \
    {                                          \
        T r = 1;                               \
        u64 b = (u64)math_##T##_abs(exponent); \
        for (u64 i = 0; i < b; ++i)            \
        {                                      \
            r *= x;                            \
        }                                      \
        if (exponent < 0)                      \
            r = 1 / r;                         \
        return r;                              \
    }

#define _IpowNoOfwT(T, ResT, LT)                                                                                       \
    static inline ResT math_##T##_power_nooverflow(T x, T exponent)                                                                   \
    {                                                                                                                   \
        LT r = 1;                                                                                                       \
        u64 b = (u64)math_##T##_abs(exponent);                                                                          \
        for (u64 i = 0; i < b; ++i)                                                                                     \
        {                                                                                                               \
            r *= (x);                                                                                                   \
            if (r != (T)r)                                                                                              \
                return (ResT){                                                                                          \
                    .value = 0,                                                                                         \
                    .error = X_ERR_EXT("math", "math_T_power_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"), \
                };                                                                                                      \
        }                                                                                                               \
        if (exponent < 0)                                                                                               \
            r = 1 / r;                                                                                                  \
        return (ResT){                                                                                                  \
            .value = (T)r,                                                                                              \
            .error = X_ERR_OK,                                                                                          \
        };                                                                                                              \
    }

// U8 ==========================================================================
_addT(u8)
_addNoOfwT(u8, ResultU8, u16)

_substractT(u8)
_substractNoOfwT(u8, ResultU8, u16)

_multiplyT(u8)
_multiplyNoOfwT(u8, ResultU8, u16)

_divideT(u8, ResultU8)
_divideNoOfwT(u8, ResultU8, u16)

_UpowT(u8)
_UpowNoOfwT(u8, ResultU8, u16)

// I8 ==========================================================================
_addT(i8)
_addNoOfwT(i8, ResultI8, i16)

_substractT(i8)
_substractNoOfwT(i8, ResultI8, i16)

_multiplyT(i8)
_multiplyNoOfwT(i8, ResultI8, i16)

_divideT(i8, ResultI8)
_divideNoOfwT(i8, ResultI8, i16)

_IabsT(i8)
_IpowT(i8)
_IpowNoOfwT(i8, ResultI8, i16)

// U16 =========================================================================
_addT(u16)
_addNoOfwT(u16, ResultU16, u32)

_substractT(u16)
_substractNoOfwT(u16, ResultU16, u32)

_multiplyT(u16)
_multiplyNoOfwT(u16, ResultU16, u32)

_divideT(u16, ResultU16)
_divideNoOfwT(u16, ResultU16, u32)

_UpowT(u16)
_UpowNoOfwT(u16, ResultU16, u32)

// I16 =========================================================================
_addT(i16)
_addNoOfwT(i16, ResultI16, i32)

_substractT(i16)
_substractNoOfwT(i16, ResultI16, i32)

_multiplyT(i16)
_multiplyNoOfwT(i16, ResultI16, i32)

_divideT(i16, ResultI16)
_divideNoOfwT(i16, ResultI16, i32)

_IabsT(i16)
_IpowT(i16)
_IpowNoOfwT(i16, ResultI16, i32)

// U32 =========================================================================
_addT(u32)
_addNoOfwT(u32, ResultU32, u64)

_substractT(u32)
_substractNoOfwT(u32, ResultU32, u64)

_multiplyT(u32)
_multiplyNoOfwT(u32, ResultU32, u64)

_divideT(u32, ResultU32)
_divideNoOfwT(u32, ResultU32, u64)

_UpowT(u32)
_UpowNoOfwT(u32, ResultU32, u64)

// I32 =========================================================================
_addT(i32)
_addNoOfwT(i32, ResultI32, i64)

_substractT(i32)
_substractNoOfwT(i32, ResultI32, i64)

_multiplyT(i32)
_multiplyNoOfwT(i32, ResultI32, i64)

_divideT(i32, ResultI32)
_divideNoOfwT(i32, ResultI32, i64)

_IabsT(i32)
_IpowT(i32)
_IpowNoOfwT(i32, ResultI32, i64)

// U64 =========================================================================
_addT(u64)

static inline ResultU64 math_u64_add_nooverflow(u64 a, u64 b)
{
    u64 res = a + b;

    if (res < a || res < b)
        return (ResultU64){
            .value = 0,
            .error = X_ERR_EXT("math", "math_u64_add_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
        };

    return (ResultU64){
        .value = res,
        .error = X_ERR_OK,
    };
}

_substractT(u64)

static inline ResultU64 math_u64_substract_nooverflow(u64 a, u64 b)
{
    if (b > a)
        return (ResultU64){
            .value = 0,
            .error = X_ERR_EXT("math", "math_u64_substract_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
        };

    return (ResultU64){
        .value = a - b,
        .error = X_ERR_OK,
    };
}

_multiplyT(u64)

static inline ResultU64 math_u64_multiply_nooverflow(u64 a, u64 b)
{
    if (a == 0 || b == 0)
    {
        return (ResultU64){
            .value = 0,
            .error = X_ERR_OK,
        };
    }

    if (a > MaxVals.U64 / b)
    {
        return (ResultU64){
            .value = 0,
            .error = X_ERR_EXT("math", "math_u64_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
        };
    }

    return (ResultU64){
        .value = a * b,
        .error = X_ERR_OK,
    };
}

_divideT(u64, ResultU64)

static inline ResultU64 math_u64_divide_nooverflow(u64 a, u64 b)
{
    if (b == 0)
    {
        return (ResultU64){
            .value = 0,
            .error = X_ERR_EXT("math", "math_u64_divide_nooverflow", ERR_INVALID_PARAMETER, "division by 0"),
        };
    }

    return (ResultU64){
        .value = a / b,
        .error = X_ERR_OK,
    };
}

_UpowT(u64)

static inline ResultU64 math_u64_power_nooverflow(u64 x, u64 exponent)
{
    u64 r = 1;
    for (u64 i = 0; i < exponent; ++i)
    {
        if (x != 0 && r > MaxVals.U64 / x)
            return (ResultU64){
                .value = 0,
                .error = X_ERR_EXT("math", "math_u64_power_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
            };
        r *= x;
    }

    return (ResultU64){
        .value = r,
        .error = X_ERR_OK,
    };
}

// U64 =========================================================================
_addT(i64)

static inline ResultI64 math_i64_add_nooverflow(i64 a, i64 b)
{
    if (((b > 0) && (a > MaxVals.I64_MAX - b)) ||
        ((b < 0) && (a < MaxVals.I64_MIN - b)))
    {
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("math", "math_i64_add_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
        };
    }

    return (ResultI64){
        .value = a + b,
        .error = X_ERR_OK,
    };
}

_substractT(i64)

static inline ResultI64 math_i64_substract_nooverflow(i64 a, i64 b)
{
    if (((b < 0) && (a > MaxVals.I64_MAX + b)) ||
        ((b > 0) && (a < MaxVals.I64_MIN + b)))
    {
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("math", "math_i64_substract_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
        };
    }

    return (ResultI64){
        .value = a - b,
        .error = X_ERR_OK,
    };
}

_multiplyT(i64)

static inline ResultI64 math_i64_multiply_nooverflow(i64 a, i64 b)
{
    if (a == 0 || b == 0)
    {
        return (ResultI64){
            .value = 0,
            .error = X_ERR_OK,
        };
    }

    if (a > 0)
    {
        if (b > 0)
        {
            if (a > MaxVals.I64_MAX / b)
                return (ResultI64){
                    .value = 0,
                    .error = X_ERR_EXT("math", "math_i64_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
                };
        }
        else
        {
            if (b < MaxVals.I64_MIN / a)
                return (ResultI64){
                    .value = 0,
                    .error = X_ERR_EXT("math", "math_i64_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
                };
        }
    }
    else
    {
        if (b > 0)
        {
            if (a < MaxVals.I64_MIN / b)
                return (ResultI64){
                    .value = 0,
                    .error = X_ERR_EXT("math", "math_i64_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
                };
        }
        else
        {
            if (a != 0 && b < MaxVals.I64_MAX / a)
                return (ResultI64){
                    .value = 0,
                    .error = X_ERR_EXT("math", "math_i64_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
                };
        }
    }

    return (ResultI64){
        .value = a * b,
        .error = X_ERR_OK,
    };
}

_divideT(i64, ResultI64)

static inline ResultI64 math_i64_divide_nooverflow(i64 a, i64 b)
{
    if (b == 0)
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("math", "math_i64_divide_nooverflow", ERR_INVALID_PARAMETER, "division by 0"),
        };

    if (a == MaxVals.I64_MIN && b == -1)
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("math", "math_i64_divide_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
        };

    return (ResultI64){
        .value = a / b,
        .error = X_ERR_OK,
    };
}

_IabsT(i64)
_IpowT(i64)

static inline ResultI64 math_i64_power_nooverflow(i64 x, i64 exponent)
{
    if (exponent < 0)
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("math", "math_i64_power_nooverflow", ERR_INVALID_PARAMETER, "negative exponent"),
        };

    if (x == 0 && exponent == 0)
        return (ResultI64){
            .value = 1,
            .error = X_ERR_OK,
        };

    i64 r = 1;

    for (i64 i = 0; i < exponent; ++i)
    {
        if (x == 0)
        {
            r = 0;
            break;
        }

        ResultI64 mulRes = math_i64_multiply_nooverflow(r, x);
        if (mulRes.error.code != ERR_OK)
            return mulRes;
        r = mulRes.value;
    }

    return (ResultI64){
        .value = r,
        .error = X_ERR_OK,
    };
}


// F32 =========================================================================
_addT(f32)
_substractT(f32)
_multiplyT(f32)
_divideT(f32, ResultF32)
_IabsT(f32)
_IpowT(f32)
_FroundT(f32, i64)

// F64 =========================================================================
_addT(f64)
_substractT(f64)
_multiplyT(f64)
_divideT(f64, ResultF64)
_IabsT(f64)
_IpowT(f64)
_FroundT(f64, i64)
