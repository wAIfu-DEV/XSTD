#pragma once

#include "stdlib.h"
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
const Allocator c_allocator = {
    ._internalState = NULL,
    .alloc = &__c_alloc_malloc,
    .free = &__c_alloc_free,
    .realloc = &__c_alloc_realloc,
};
