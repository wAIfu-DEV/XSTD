#pragma once

// Implementation of Allocator using Cstd malloc, realloc and free

#include "xstd_alloc.h"

#include "stdlib.h"

static void *_c_alloc_malloc(Allocator *a, u64 size)
{
    (void)a;
    return malloc(size);
}

static void *_c_alloc_realloc(Allocator *a, void *block, u64 size)
{
    (void)a;
    return realloc(block, size);
}

static void _c_alloc_free(Allocator *a, void *block)
{
    (void)a;
    free(block);
}

static Allocator _c_allocator = {
    ._internalState = NULL,
    .alloc = &_c_alloc_malloc,
    .realloc = &_c_alloc_realloc,
    .free = &_c_alloc_free,
};
