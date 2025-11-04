#pragma once

// Implementation of Allocator using Cstd malloc, realloc and free

#include "xstd_alloc.h"

#include "stdlib.h"

void *__c_alloc_malloc(Allocator *this, u64 size)
{
    (void)this;
    return malloc(size);
}

void *__c_alloc_realloc(Allocator *this, void *block, u64 size)
{
    (void)this;
    return realloc(block, size);
}

void __c_alloc_free(Allocator *this, void *block)
{
    (void)this;
    free(block);
}

/**
 * @brief Default allocator making use of stdlib's malloc, realloc and free.
 *
 * ```c
 * HeapStr newStr = c_allocator.alloc(&c_allocator, sizeof(i8) * 64);
 * ```
 * @return Allocator
 */
Allocator c_allocator = {
    ._internalState = NULL,
    .alloc = &__c_alloc_malloc,
    .free = &__c_alloc_free,
    .realloc = &__c_alloc_realloc,
};
