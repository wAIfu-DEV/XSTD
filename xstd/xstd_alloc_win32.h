#pragma once

// Implementation of Allocator using win32

#include "xstd_alloc.h"
#include "xstd_win32.h"

typedef struct {
    _w32_handle procHeapHandle;
} _win32_alloc_state_t;

static _win32_alloc_state_t _win32_alloc_state = {
    .procHeapHandle = NULL,
};

static void *_win32_malloc(Allocator *a, u64 size)
{
    (void)a;
    return HeapAlloc(_win32_alloc_state.procHeapHandle, 0, size);
}

static void *_win32_realloc(Allocator *a, void *block, u64 size)
{
    (void)a;
    return HeapReAlloc(_win32_alloc_state.procHeapHandle, 0, block, size);
}

static void _win32_free(Allocator *a, void *block)
{
    (void)a;
    HeapFree(_win32_alloc_state.procHeapHandle, 0, block);
}

static Allocator _win32_allocator = {
    ._internalState = &_win32_alloc_state,
    .alloc = &_win32_malloc,
    .realloc = &_win32_realloc,
    .free = &_win32_free,
};
