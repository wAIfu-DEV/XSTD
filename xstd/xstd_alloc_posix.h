#pragma once

#include "xstd_alloc.h"

#include "sys/mman.h"
#include "unistd.h"

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

typedef struct
{
    u64 pageSize;
} _posix_alloc_state_t;

typedef struct
{
    u64 request_size;
    u64 mapping_size;
} _posix_alloc_header_t;

static _posix_alloc_state_t _posix_alloc_state = {
    .pageSize = 0,
};

static u64 _posix_align_up(u64 value, u64 alignment)
{
    if (alignment == 0)
    {
        return value;
    }

    u64 remainder = value % alignment;
    if (remainder == 0)
    {
        return value;
    }

    return value + (alignment - remainder);
}

static u64 _posix_get_page_size(void)
{
    if (_posix_alloc_state.pageSize == 0)
    {
        long result = sysconf(_SC_PAGESIZE);
        if (result <= 0)
        {
            result = 4096;
        }
        _posix_alloc_state.pageSize = (u64)result;
    }

    return _posix_alloc_state.pageSize;
}

static _posix_alloc_header_t *_posix_header_from_block(void *block)
{
    return ((_posix_alloc_header_t *)block) - 1;
}

static void _posix_copy_memory(void *dst, const void *src, u64 size)
{
    u8 *d = (u8 *)dst;
    const u8 *s = (const u8 *)src;

    while (size--)
    {
        *d++ = *s++;
    }
}

static u64 _posix_min_u64(u64 a, u64 b)
{
    return (a < b) ? a : b;
}

static void *_posix_alloc(Allocator *a, u64 size)
{
    (void)a;

    if (size == 0)
    {
        return NULL;
    }

    u64 pageSize = _posix_get_page_size();
    u64 totalSize = sizeof(_posix_alloc_header_t) + size;
    u64 mappingSize = _posix_align_up(totalSize, pageSize);

    void *base = mmap(NULL, mappingSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base == MAP_FAILED)
    {
        return NULL;
    }

    _posix_alloc_header_t *header = (_posix_alloc_header_t *)base;
    header->request_size = size;
    header->mapping_size = mappingSize;

    return (void *)(header + 1);
}

static void *_posix_realloc(Allocator *a, void *block, u64 newSize)
{
    (void)a;

    if (block == NULL)
    {
        return _posix_alloc(a, newSize);
    }

    if (newSize == 0)
    {
        _posix_alloc_header_t *header = _posix_header_from_block(block);
        munmap((void *)header, header->mapping_size);
        return NULL;
    }

    _posix_alloc_header_t *oldHeader = _posix_header_from_block(block);
    u64 pageSize = _posix_get_page_size();
    u64 totalSize = sizeof(_posix_alloc_header_t) + newSize;
    u64 newMappingSize = _posix_align_up(totalSize, pageSize);

#ifdef _linux__
    if (newMappingSize == oldHeader->mapping_size)
    {
        oldHeader->request_size = newSize;
        return block;
    }

    void *base = (void *)oldHeader;
    void *remapped = mremap(base, oldHeader->mapping_size, newMappingSize, MREMAP_MAYMOVE);
    if (remapped != MAP_FAILED)
    {
        _posix_alloc_header_t *newHeader = (_posix_alloc_header_t *)remapped;
        newHeader->request_size = newSize;
        newHeader->mapping_size = newMappingSize;
        return (void *)(newHeader + 1);
    }
#endif

    void *newBlock = _posix_alloc(a, newSize);
    if (newBlock == NULL)
    {
        return NULL;
    }

    u64 copySize = _posix_min_u64(oldHeader->request_size, newSize);
    if (copySize > 0)
    {
        _posix_copy_memory(newBlock, block, copySize);
    }

    munmap((void *)oldHeader, oldHeader->mapping_size);

    return newBlock;
}

static void _posix_free(Allocator *a, void *block)
{
    (void)a;

    if (block == NULL)
    {
        return;
    }

    _posix_alloc_header_t *header = _posix_header_from_block(block);
    munmap((void *)header, header->mapping_size);
}

static Allocator _posix_allocator = {
    ._internalState = &_posix_alloc_state,
    .alloc = &_posix_alloc,
    .realloc = &_posix_realloc,
    .free = &_posix_free,
};
