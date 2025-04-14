#pragma once

#include "xstd_coretypes.h"
#include "xstd_err.h"

// If `error` != `ERR_OK` then `value` is invalid. Do not access without checking.
//
// Memory is not owned by the caller and should not be freed. Use `ResultOwned`
// instead if the caller should own the memory.
typedef struct _result_void
{
    void *value;
    Error error;
} Result;

// If `error` != `ERR_OK` then `value` is invalid. Do not access without checking.
//
// `value` should be freed if valid.
typedef struct _result_void_owned
{
    void *value;
    Error error;
} ResultOwned;

// If `error` != `ERR_OK` then `value` is invalid. Do not access without checking.
typedef struct _result_int
{
    i64 value;
    Error error;
} ResultI64;

// If `error` != `ERR_OK` then `value` is invalid. Do not access without checking.
typedef struct _result_uint
{
    u64 value;
    Error error;
} ResultU64;

// If `error` != `ERR_OK` then `value` is invalid. Do not access without checking.
typedef struct _result_float
{
    f64 value;
    Error error;
} ResultF64;

// If `error` != `ERR_OK` then `value` is invalid. Do not access without checking.
typedef struct _result_string
{
    String value;
    Error error;
} ResultStr;

// If `error` != `ERR_OK` then `value` is invalid. Do not access without checking.
//
// HeapStr `value` should be freed after use if valid.
typedef struct _result_owned_string
{
    HeapStr value;
    Error error;
} ResultOwnedStr;

// If `error` != `ERR_OK` then `value` is invalid. Do not access without checking.
typedef struct _result_buffer
{
    Buffer value;
    Error error;
} ResultBuffer;

// If `error` != `ERR_OK` then `value` is invalid. Do not access without checking.
//
// HeapBuff `value` should be freed after use if valid.
typedef struct _result_buffer_owned
{
    HeapBuff value;
    Error error;
} ResultOwnedBuff;

// If `error` != `ERR_OK` then `value` is invalid. Do not access without checking.
typedef struct _result_buffer_const
{
    ConstBuff value;
    Error error;
} ResultConstBuff;
