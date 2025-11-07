#pragma once

#include "xstd_core.h"
#include "xstd_error.h"

// Struct with methods to manipulate heap memory, implementations can be created
// by overwriting those methods. Get the default allocator using `default_allocator()`
typedef struct _allocator_t
{
    void *_internalState;
    void *(*alloc)(struct _allocator_t *a, u64 allocSize);
    void *(*realloc)(struct _allocator_t *a, void *block, u64 newSize);
    void (*free)(struct _allocator_t *a, void *block);
} Allocator;

// If `error` != `ERR_OK` then `value` is invalid. Do not access without checking.
typedef struct _result_allocator
{
    Allocator value;
    Error error;
} ResultAllocator;
