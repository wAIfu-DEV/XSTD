#pragma once

#include "xstd_core.h"
#include "xstd_error.h"

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

#define __addT(T)              \
    T math_##T##_add(T a, T b) \
    {                          \
        return a + b;          \
    }

#define __addNoOfwT(T, ResT, LT)                                                                                  \
    ResT math_##T##_add_nooverflow(T a, T b)                                                                      \
    {                                                                                                             \
        LT res = (LT)a + (LT)b;                                                                                   \
                                                                                                                  \
        if (res != (T)res)                                                                                        \
            return (ResT){                                                                                        \
                .value = 0,                                                                                       \
                .error = X_ERR_EXT("math", "math_T_add_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"), \
            };                                                                                                    \
                                                                                                                  \
        return (ResT){                                                                                            \
            .value = (T)res,                                                                                      \
            .error = X_ERR_OK,                                                                                    \
        };                                                                                                        \
    }

#define __substractT(T)              \
    T math_##T##_substract(T a, T b) \
    {                                \
        return a - b;                \
    }

#define __substractNoOfwT(T, ResT, LT)                                                                                  \
    ResT math_##T##_substract_nooverflow(T a, T b)                                                                      \
    {                                                                                                                   \
        LT res = (LT)a - (LT)b;                                                                                         \
                                                                                                                        \
        if (res != (T)res)                                                                                              \
            return (ResT){                                                                                              \
                .value = 0,                                                                                             \
                .error = X_ERR_EXT("math", "math_T_substract_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"), \
            };                                                                                                          \
                                                                                                                        \
        return (ResT){                                                                                                  \
            .value = (T)res,                                                                                            \
            .error = X_ERR_OK,                                                                                          \
        };                                                                                                              \
    }

#define __multiplyT(T)                  \
    T math_##T##_multiply(T x, T mulBy) \
    {                                   \
        return x * mulBy;               \
    }

#define __multiplyNoOfwT(T, ResT, LT)                                                                                  \
    ResT math_##T##_multiply_nooverflow(T a, T b)                                                                      \
    {                                                                                                                  \
        LT res = (LT)a * (LT)b;                                                                                        \
                                                                                                                       \
        if (res != (T)res)                                                                                             \
            return (ResT){                                                                                             \
                .value = 0,                                                                                            \
                .error = X_ERR_EXT("math", "math_T_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"), \
            };                                                                                                         \
                                                                                                                       \
        return (ResT){                                                                                                 \
            .value = (T)res,                                                                                           \
            .error = X_ERR_OK,                                                                                         \
        };                                                                                                             \
    }

#define __divideT(T, ResT)                                                                                \
    ResT math_##T##_divide(T x, T divBy)                                                                  \
    {                                                                                                     \
        if (divBy == 0)                                                                                   \
            return (ResT){                                                                                \
                .value = 0,                                                                               \
                .error = X_ERR_EXT("math", "math_T_divide", ERR_INVALID_PARAMETER, "division by 0"), \
            };                                                                                            \
                                                                                                          \
        return (ResT){                                                                                    \
            .value = x / divBy,                                                                           \
            .error = X_ERR_OK,                                                                            \
        };                                                                                                \
    }

#define __divideNoOfwT(T, ResT, LT)                                                                                  \
    ResT math_##T##_divide_nooverflow(T x, T divBy)                                                                  \
    {                                                                                                                \
        if (divBy == 0)                                                                                              \
            return (ResT){                                                                                           \
                .value = 0,                                                                                          \
                .error = X_ERR_EXT("math", "math_T_divide_nooverflow", ERR_INVALID_PARAMETER, "division by 0"), \
            };                                                                                                       \
                                                                                                                     \
        LT res = (LT)x / (LT)divBy;                                                                                  \
                                                                                                                     \
        if (res != (T)res)                                                                                           \
            return (ResT){                                                                                           \
                .value = 0,                                                                                          \
                .error = X_ERR_EXT("math", "math_T_divide_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"), \
            };                                                                                                       \
                                                                                                                     \
        return (ResT){                                                                                               \
            .value = (T)res,                                                                                         \
            .error = X_ERR_OK,                                                                                       \
        };                                                                                                           \
    }

#define __IabsT(T)                        \
    T math_##T##_abs(T x)                 \
    {                                     \
        return (T)((x >= (T)0) ? x : -x); \
    }

#define __UpowT(T)                      \
    T math_##T##_power(T x, T exponent) \
    {                                   \
        T r = 1;                        \
        T b = (exponent);               \
        for (T i = 0; i < b; ++i)       \
        {                               \
            r *= (x);                   \
        }                               \
        return r;                       \
    }

#define __UpowNoOfwT(T, ResT, LT)                                                                                       \
    ResT math_##T##_power_nooverflow(T x, T exponent)                                                                   \
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

#define __IpowT(T)                             \
    T math_##T##_power(T x, T exponent)        \
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

#define __IpowNoOfwT(T, ResT, LT)                                                                                       \
    ResT math_##T##_power_nooverflow(T x, T exponent)                                                                   \
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
__addT(u8)
__addNoOfwT(u8, ResultU8, u16)

__substractT(u8)
__substractNoOfwT(u8, ResultU8, u16)

__multiplyT(u8)
__multiplyNoOfwT(u8, ResultU8, u16)

__divideT(u8, ResultU8)
__divideNoOfwT(u8, ResultU8, u16)

__UpowT(u8)
__UpowNoOfwT(u8, ResultU8, u16)

// I8 ==========================================================================
__addT(i8)
__addNoOfwT(i8, ResultI8, i16)

__substractT(i8)
__substractNoOfwT(i8, ResultI8, i16)

__multiplyT(i8)
__multiplyNoOfwT(i8, ResultI8, i16)

__divideT(i8, ResultI8)
__divideNoOfwT(i8, ResultI8, i16)

__IabsT(i8)
__IpowT(i8)
__IpowNoOfwT(i8, ResultI8, i16)

// U16 =========================================================================
__addT(u16)
__addNoOfwT(u16, ResultU16, u32)

__substractT(u16)
__substractNoOfwT(u16, ResultU16, u32)

__multiplyT(u16)
__multiplyNoOfwT(u16, ResultU16, u32)

__divideT(u16, ResultU16)
__divideNoOfwT(u16, ResultU16, u32)

__UpowT(u16)
__UpowNoOfwT(u16, ResultU16, u32)

// I16 =========================================================================
__addT(i16)
__addNoOfwT(i16, ResultI16, i32)

__substractT(i16)
__substractNoOfwT(i16, ResultI16, i32)

__multiplyT(i16)
__multiplyNoOfwT(i16, ResultI16, i32)

__divideT(i16, ResultI16)
__divideNoOfwT(i16, ResultI16, i32)

__IabsT(i16)
__IpowT(i16)
__IpowNoOfwT(i16, ResultI16, i32)

// U32 =========================================================================
__addT(u32)
__addNoOfwT(u32, ResultU32, u64)

__substractT(u32)
__substractNoOfwT(u32, ResultU32, u64)

__multiplyT(u32)
__multiplyNoOfwT(u32, ResultU32, u64)

__divideT(u32, ResultU32)
__divideNoOfwT(u32, ResultU32, u64)

__UpowT(u32)
__UpowNoOfwT(u32, ResultU32, u64)

// I32 =========================================================================
__addT(i32)
__addNoOfwT(i32, ResultI32, i64)

__substractT(i32)
__substractNoOfwT(i32, ResultI32, i64)

__multiplyT(i32)
__multiplyNoOfwT(i32, ResultI32, i64)

__divideT(i32, ResultI32)
__divideNoOfwT(i32, ResultI32, i64)

__IabsT(i32)
__IpowT(i32)
__IpowNoOfwT(i32, ResultI32, i64)

// U64 =========================================================================
__addT(u64)

ResultU64 math_u64_add_nooverflow(u64 a, u64 b)
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

__substractT(u64)

ResultU64 math_u64_substract_nooverflow(u64 a, u64 b)
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

__multiplyT(u64)

ResultU64 math_u64_multiply_nooverflow(u64 a, u64 b)
{
    if (a == 0 || b == 0)
    {
        return (ResultU64){
            .value = 0,
            .error = X_ERR_OK,
        };
    }

    if (a > U64_MAXVAL / b)
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

__divideT(u64, ResultU64)

ResultU64 math_u64_divide_nooverflow(u64 a, u64 b)
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

__UpowT(u64)

ResultU64 math_u64_power_nooverflow(u64 x, u64 exponent)
{
    u64 r = 1;
    for (u64 i = 0; i < exponent; ++i)
    {
        if (x != 0 && r > U64_MAXVAL / x)
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
__addT(i64)

ResultI64 math_i64_add_nooverflow(i64 a, i64 b)
{
    if (((b > 0) && (a > I64_MAXVAL - b)) ||
        ((b < 0) && (a < I64_MINVAL - b)))
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

__substractT(i64)

ResultI64 math_i64_substract_nooverflow(i64 a, i64 b)
{
    if (((b < 0) && (a > I64_MAXVAL + b)) ||
        ((b > 0) && (a < I64_MINVAL + b)))
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

__multiplyT(i64)

ResultI64 math_i64_multiply_nooverflow(i64 a, i64 b)
{
    if (a == 0 || b == 0)
    {
        return (ResultI64){
            .value = 0,
            .error = X_ERR_OK,
        };
    }

    if (a == -1 && b == I64_MINVAL)
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("math", "math_i64_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
        };

    if (b == -1 && a == I64_MINVAL)
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("math", "math_i64_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
        };

    i64 res = a * b;

    if (res / b != a)
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("math", "math_i64_multiply_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
        };

    return (ResultI64){
        .value = res,
        .error = X_ERR_OK,
    };
}

__divideT(i64, ResultI64)

ResultI64 math_i64_divide_nooverflow(i64 a, i64 b)
{
    if (b == 0)
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("math", "math_i64_divide_nooverflow", ERR_INVALID_PARAMETER, "division by 0"),
        };

    if (a == I64_MINVAL && b == -1)
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("math", "math_i64_divide_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
        };

    return (ResultI64){
        .value = a / b,
        .error = X_ERR_OK,
    };
}

__IabsT(i64)
__IpowT(i64)

ResultI64 math_i64_power_nooverflow(i64 x, i64 exponent)
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
        if ((x > 0 && (r > I64_MAXVAL / x || r < I64_MINVAL / x)) ||
            (x < 0 && (r < I64_MINVAL / x || r > I64_MAXVAL / x)))
            return (ResultI64){
                .value = 0,
                .error = X_ERR_EXT("math", "math_i64_power_nooverflow", ERR_WOULD_OVERFLOW, "integer overflow"),
            };
    }

    return (ResultI64){
        .value = r,
        .error = X_ERR_OK,
    };
}


// F32 =========================================================================
__addT(f32)
__substractT(f32)
__multiplyT(f32)
__divideT(f32, ResultF32)
__IabsT(f32)
__IpowT(f32)

// F64 =========================================================================
__addT(f64)
__substractT(f64)
__multiplyT(f64)
__divideT(f64, ResultF64)
__IabsT(f64)
__IpowT(f64)
