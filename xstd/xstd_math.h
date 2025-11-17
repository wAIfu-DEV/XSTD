#pragma once

#include "xstd_core.h"
#include "xstd_errcode.h"
#include "xstd_error.h"
#include "xstd_result.h"

result_define(u8, U8);
result_define(i8, I8);
result_define(u16, U16);
result_define(i16, I16);
result_define(u32, U32);
result_define(i32, I32);
result_define(f32, F32);

#define _addT(T)                             \
    static inline T math_##T##_add(T a, T b) \
    {                                        \
        return a + b;                        \
    }

#define _addNoOfwT(T, ResT, LT)                                                                                   \
    static inline ResT math_##T##_add_nooverflow(T a, T b)                                                        \
    {                                                                                                             \
        LT res = (LT)a + (LT)b;                                                                                   \
                                                                                                                  \
        if (res != (T)res)                                                                                        \
            return (ResT){                                                                                        \
                .isErr = 1,                                                                                       \
                .err = X_ERR_EXT("math", "math_T_add_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),        \
            };                                                                                                    \
                                                                                                                  \
        return (ResT){                                                                                            \
            .isErr = 0,                                                                                           \
            .value = (T)res,                                                                                      \
        };                                                                                                        \
    }

#define _substractT(T)                             \
    static inline T math_##T##_substract(T a, T b) \
    {                                              \
        return a - b;                              \
    }

#define _substractNoOfwT(T, ResT, LT)                                                                                   \
    static inline ResT math_##T##_substract_nooverflow(T a, T b)                                                        \
    {                                                                                                                   \
        LT res = (LT)a - (LT)b;                                                                                         \
                                                                                                                        \
        if (res != (T)res)                                                                                              \
            return (ResT){                                                                                              \
                .isErr = 1,                                                                                             \
                .err = X_ERR_EXT("math", "math_T_substract_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),        \
            };                                                                                                          \
                                                                                                                        \
        return (ResT){                                                                                                  \
            .isErr = 0,                                                                                                 \
            .value = (T)res,                                                                                            \
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
                .isErr = 1,                                                                                            \
                .err = X_ERR_EXT("math", "math_T_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),        \
            };                                                                                                         \
                                                                                                                       \
        return (ResT){                                                                                                 \
            .isErr = 0,                                                                                                \
            .value = (T)res,                                                                                           \
        };                                                                                                             \
    }

#define _divideT(T, ResT)                                                                                 \
    static inline ResT math_##T##_divide(T x, T divBy)                                                    \
    {                                                                                                     \
        if ((T)divBy == (T)0)                                                                             \
            return (ResT){                                                                                \
                .isErr = 1,                                                                               \
                .err = X_ERR_EXT("math", "math_T_divide", ERR_INVALID_PARAMETER, "division by 0"),        \
            };                                                                                            \
                                                                                                          \
        return (ResT){                                                                                    \
            .isErr = 0,                                                                                   \
            .value = (T)((T)x / (T)divBy),                                                                \
        };                                                                                                \
    }

#define _divideNoOfwT(T, ResT, LT)                                                                                   \
    static inline ResT math_##T##_divide_nooverflow(T x, T divBy)                                                    \
    {                                                                                                                \
        if (divBy == 0)                                                                                              \
            return (ResT){                                                                                           \
                .isErr = 1,                                                                                          \
                .err = X_ERR_EXT("math", "math_T_divide_nooverflow", ERR_INVALID_PARAMETER, "division by 0"),        \
            };                                                                                                       \
                                                                                                                     \
        LT res = (LT)x / (LT)divBy;                                                                                  \
                                                                                                                     \
        if (res != (T)res)                                                                                           \
            return (ResT){                                                                                           \
                .isErr = 1,                                                                                          \
                .err = X_ERR_EXT("math", "math_T_divide_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),        \
            };                                                                                                       \
                                                                                                                     \
        return (ResT){                                                                                               \
            .isErr = 0,                                                                                              \
            .value = (T)res,                                                                                         \
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

#define _UpowT(T)                                     \
    static inline T math_##T##_power(T x, T exponent) \
    {                                                 \
        T r = 1;                                      \
        T b = (exponent);                             \
        for (T i = 0; i < b; ++i)                     \
        {                                             \
            r *= (x);                                 \
        }                                             \
        return r;                                     \
    }

#define _UpowNoOfwT(T, ResT, LT)                                                                                        \
    static inline ResT math_##T##_power_nooverflow(T x, T exponent)                                                     \
    {                                                                                                                   \
        LT r = 1;                                                                                                       \
        LT b = (exponent);                                                                                              \
        for (LT i = 0; i < b; ++i)                                                                                      \
        {                                                                                                               \
            r *= (x);                                                                                                   \
            if (r != (T)r)                                                                                              \
                return (ResT){                                                                                          \
                    .isErr = 1,                                                                                         \
                    .err = X_ERR_EXT("math", "math_T_power_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),        \
                };                                                                                                      \
        }                                                                                                               \
        return (ResT){                                                                                                  \
            .isErr = 0,                                                                                                 \
            .value = (T)r,                                                                                              \
        };                                                                                                              \
    }

#define _IpowT(T)                                     \
    static inline T math_##T##_power(T x, T exponent) \
    {                                                 \
        T r = 1;                                      \
        u64 b = (u64)math_##T##_abs(exponent);        \
        for (u64 i = 0; i < b; ++i)                   \
        {                                             \
            r *= x;                                   \
        }                                             \
        if (exponent < 0)                             \
            r = 1 / r;                                \
        return r;                                     \
    }

#define _IpowNoOfwT(T, ResT, LT)                                                                                        \
    static inline ResT math_##T##_power_nooverflow(T x, T exponent)                                                     \
    {                                                                                                                   \
        LT r = 1;                                                                                                       \
        u64 b = (u64)math_##T##_abs(exponent);                                                                          \
        for (u64 i = 0; i < b; ++i)                                                                                     \
        {                                                                                                               \
            r *= (x);                                                                                                   \
            if (r != (T)r)                                                                                              \
                return (ResT){                                                                                          \
                    .isErr = 1,                                                                                         \
                    .err = X_ERR_EXT("math", "math_T_power_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),        \
                };                                                                                                      \
        }                                                                                                               \
        if (exponent < 0)                                                                                               \
            r = 1 / r;                                                                                                  \
        return (ResT){                                                                                                  \
            .isErr = 0,                                                                                                 \
            .value = (T)r,                                                                                              \
        };                                                                                                              \
    }

// U8 ==========================================================================
_addT(u8)
_addNoOfwT(u8, result_type(U8), u16)

_substractT(u8)
_substractNoOfwT(u8, result_type(U8), u16)

_multiplyT(u8)
_multiplyNoOfwT(u8, result_type(U8), u16)

_divideT(u8, result_type(U8))
_divideNoOfwT(u8, result_type(U8), u16)

_UpowT(u8)
_UpowNoOfwT(u8, result_type(U8), u16)

// I8 ==========================================================================
_addT(i8)
_addNoOfwT(i8, result_type(I8), i16)

_substractT(i8)
_substractNoOfwT(i8, result_type(I8), i16)

_multiplyT(i8)
_multiplyNoOfwT(i8, result_type(I8), i16)

_divideT(i8, result_type(I8))
_divideNoOfwT(i8, result_type(I8), i16)

_IabsT(i8)
_IpowT(i8)
_IpowNoOfwT(i8, result_type(I8), i16)

// U16 =========================================================================
_addT(u16)
_addNoOfwT(u16, result_type(U16), u32)

_substractT(u16)
_substractNoOfwT(u16, result_type(U16), u32)

_multiplyT(u16)
_multiplyNoOfwT(u16, result_type(U16), u32)

_divideT(u16, result_type(U16))
_divideNoOfwT(u16, result_type(U16), u32)

_UpowT(u16)
_UpowNoOfwT(u16, result_type(U16), u32)

// I16 =========================================================================
_addT(i16)
_addNoOfwT(i16, result_type(I16), i32)

_substractT(i16)
_substractNoOfwT(i16, result_type(I16), i32)

_multiplyT(i16)
_multiplyNoOfwT(i16, result_type(I16), i32)

_divideT(i16, result_type(I16))
_divideNoOfwT(i16, result_type(I16), i32)

_IabsT(i16)
_IpowT(i16)
_IpowNoOfwT(i16, result_type(I16), i32)

// U32 =========================================================================
_addT(u32)
_addNoOfwT(u32, result_type(U32), u64)

_substractT(u32)
_substractNoOfwT(u32, result_type(U32), u64)

_multiplyT(u32)
_multiplyNoOfwT(u32, result_type(U32), u64)

_divideT(u32, result_type(U32))
_divideNoOfwT(u32, result_type(U32), u64)

_UpowT(u32)
_UpowNoOfwT(u32, result_type(U32), u64)

// I32 =========================================================================
_addT(i32)
_addNoOfwT(i32, result_type(I32), i64)

_substractT(i32)
_substractNoOfwT(i32, result_type(I32), i64)

_multiplyT(i32)
_multiplyNoOfwT(i32, result_type(I32), i64)

_divideT(i32, result_type(I32))
_divideNoOfwT(i32, result_type(I32), i64)

_IabsT(i32)
_IpowT(i32)
_IpowNoOfwT(i32, result_type(I32), i64)

// U64 =========================================================================
_addT(u64)

static inline result_type(U64) math_u64_add_nooverflow(u64 a, u64 b)
{
    u64 res = a + b;

    if (res < a || res < b)
        return result_err(U64, X_ERR_EXT("math", "math_u64_add_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"));

    return result_ok(U64, res);
}

_substractT(u64)

static inline result_type(U64) math_u64_substract_nooverflow(u64 a, u64 b)
{
    if (b > a)
        return result_err(U64, X_ERR_EXT("math", "math_u64_substract_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"));

    return result_ok(U64, a - b);
}

_multiplyT(u64)

static inline result_type(U64) math_u64_multiply_nooverflow(u64 a, u64 b)
{
    if (a == 0 || b == 0)
    {
        return result_ok(U64, 0);
    }

    if (a > EnumMaxVal.U64 / b)
    {
        return result_err(U64, X_ERR_EXT("math", "math_u64_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"));
    }
    return result_ok(U64, a * b);
}

_divideT(u64, result_type(U64))

static inline result_type(U64) math_u64_divide_nooverflow(u64 a, u64 b)
{
    if (b == 0)
        return result_err(U64, X_ERR_EXT("math", "math_u64_divide_nooverflow", ERR_INVALID_PARAMETER, "division by 0"));

    return result_ok(U64, a / b);
}

_UpowT(u64)

static inline result_type(U64) math_u64_power_nooverflow(u64 x, u64 exponent)
{
    u64 r = 1;
    for (u64 i = 0; i < exponent; ++i)
    {
        if (x != 0 && r > EnumMaxVal.U64 / x)
            return result_err(U64, X_ERR_EXT("math", "math_u64_power_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"));

        r *= x;
    }

    return result_ok(U64, r);
}

// U64 =========================================================================
_addT(i64)

static inline result_type(I64) math_i64_add_nooverflow(i64 a, i64 b)
{
    if (((b > 0) && (a > EnumMaxVal.I64_MAX - b)) ||
        ((b < 0) && (a < EnumMaxVal.I64_MIN - b)))
    {
        return result_err(I64, X_ERR_EXT("math", "math_i64_add_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"));
    }
    return result_ok(I64, a + b);
}

_substractT(i64)

static inline result_type(I64) math_i64_substract_nooverflow(i64 a, i64 b)
{
    if (((b < 0) && (a > EnumMaxVal.I64_MAX + b)) ||
        ((b > 0) && (a < EnumMaxVal.I64_MIN + b)))
    {
        return result_err(I64, X_ERR_EXT("math", "math_i64_substract_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"));
    }
    return result_ok(I64, a - b);
}

_multiplyT(i64)

static inline result_type(I64) math_i64_multiply_nooverflow(i64 a, i64 b)
{
    if (a == 0 || b == 0)
    {
        return result_ok(I64, 0);
    }

    if (a > 0)
    {
        if (b > 0)
        {
            if (a > EnumMaxVal.I64_MAX / b)
                return result_err(I64, X_ERR_EXT("math", "math_i64_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"));
        }
        else
        {
            if (b < EnumMaxVal.I64_MIN / a)
                return result_err(I64, X_ERR_EXT("math", "math_i64_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"));
        }
    }
    else
    {
        if (b > 0)
        {
            if (a < EnumMaxVal.I64_MIN / b)
                return result_err(I64, X_ERR_EXT("math", "math_i64_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"));
        }
        else
        {
            if (a != 0 && b < EnumMaxVal.I64_MAX / a)
                return result_err(I64, X_ERR_EXT("math", "math_i64_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"));
        }
    }
    return result_ok(I64, a * b);
}

_divideT(i64, result_type(I64))

static inline result_type(I64) math_i64_divide_nooverflow(i64 a, i64 b)
{
    if (b == 0)
        return result_err(I64, X_ERR_EXT("math", "math_i64_divide_nooverflow", ERR_INVALID_PARAMETER, "division by 0"));

    if (a == EnumMaxVal.I64_MIN && b == -1)
        return result_err(I64, X_ERR_EXT("math", "math_i64_divide_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"));

    return result_ok(I64, a / b);
}

_IabsT(i64)
_IpowT(i64)

static inline result_type(I64) math_i64_power_nooverflow(i64 x, i64 exponent)
{
    if (exponent < 0)
        return result_err(I64, X_ERR_EXT("math", "math_i64_power_nooverflow", ERR_INVALID_PARAMETER, "negative exponent"));

    if (x == 0 && exponent == 0)
        return result_ok(I64, 1);

    i64 r = 1;

    for (i64 i = 0; i < exponent; ++i)
    {
        if (x == 0)
        {
            r = 0;
            break;
        }

        result_type(I64) mulRes = math_i64_multiply_nooverflow(r, x);
        if (mulRes.isErr)
            return mulRes;
        r = mulRes.value;
    }

    return result_ok(I64, r);
}


// F32 =========================================================================
_addT(f32)
_substractT(f32)
_multiplyT(f32)
_divideT(f32, result_type(F32))
_IabsT(f32)
_IpowT(f32)
_FroundT(f32, i64)

// F64 =========================================================================
_addT(f64)
_substractT(f64)
_multiplyT(f64)
_divideT(f64, result_type(F64))
_IabsT(f64)
_IpowT(f64)
_FroundT(f64, i64)
