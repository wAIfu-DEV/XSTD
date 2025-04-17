#pragma once

#include "xstd_coretypes.h"

// Struct with methods to manipulate heap memory, implementations can be created
// by overwriting those methods. Get the default allocator using `c_allocator`
typedef struct _allocator_t
{
    void *_internalState;
    void *(*alloc)(struct _allocator_t *this, u64 allocSize);
    void *(*realloc)(struct _allocator_t *this, void *block, u64 newSize);
    void (*free)(struct _allocator_t *this, void *block);
} Allocator;

// If `error` != `ERR_OK` then `value` is invalid. Do not access without checking.
typedef struct _result_allocator
{
    Allocator value;
    Error error;
} ResultAllocator;
