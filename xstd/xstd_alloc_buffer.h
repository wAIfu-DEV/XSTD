#pragma once

#include "xstd_core.h"
#include "xstd_alloc.h"
#include "xstd_error.h"
#include "xstd_buffer.h"

typedef struct _buffalloc_block
{
    u64 size;              // Total size including header
    ibool isFree;          // If this block is free
    struct _buffalloc_block *next; // Next block in the list
} _BufferBlockHeader;

typedef struct _buffalloc_state
{
    i8 *buffer;
    u64 capacity;
    _BufferBlockHeader *head;
} BufferAllocatorState;

#define _X_BUFFALLOC_HEADER_SIZE ((u64)__buffalloc_offset_to_aligned(sizeof(BufferAllocatorState)))
#define _X_BUFFALLOC_BLOCK_HEADER_SIZE ((u64)__buffalloc_offset_to_aligned(sizeof(_BufferBlockHeader)))
#define _X_BUFFALLOC_DEFAULT_ALIGN 16

u64 __buffalloc_offset_to_aligned(u64 offset)
{
    return (offset + _X_BUFFALLOC_DEFAULT_ALIGN - 1) & ~(_X_BUFFALLOC_DEFAULT_ALIGN - 1);
}

ibool __buffalloc_offset_invalid(u64 totalCapacity, u64 alignedOffset, u64 allocSize)
{
    if ((alignedOffset + allocSize) > totalCapacity)
        return true;

    if (alignedOffset > ((u64)-1) - allocSize)
        return true;

    return false;
}

static void *__buffalloc_alloc(Allocator *this, u64 size)
{
    if (!this || !this->_internalState || size == 0)
        return NULL;

    BufferAllocatorState *state = (BufferAllocatorState *)this->_internalState;
    _BufferBlockHeader *curr = state->head;

    u64 totalSize = __buffalloc_offset_to_aligned(size) + _X_BUFFALLOC_BLOCK_HEADER_SIZE;

    while (curr)
    {
        if (curr->isFree && curr->size >= totalSize)
        {
            u64 leftover = curr->size - totalSize;

            if (leftover > _X_BUFFALLOC_BLOCK_HEADER_SIZE + 16) // Enough for splitting
            {
                _BufferBlockHeader *newBlock = (_BufferBlockHeader *)((i8 *)curr + totalSize);
                newBlock->size = leftover;
                newBlock->isFree = true;
                newBlock->next = curr->next;

                curr->size = totalSize;
                curr->next = newBlock;
            }

            curr->isFree = false;
            return (i8 *)curr + _X_BUFFALLOC_BLOCK_HEADER_SIZE;
        }

        curr = curr->next;
    }

    return NULL;
}

static void __buffalloc_free(Allocator *this, void *ptr)
{
    if (!this || !ptr)
        return;

    BufferAllocatorState *state = (BufferAllocatorState *)this->_internalState;
    _BufferBlockHeader *block = (_BufferBlockHeader *)((i8 *)ptr - _X_BUFFALLOC_BLOCK_HEADER_SIZE);
    block->isFree = true;

    // Coalesce adjacent free blocks
    _BufferBlockHeader *curr = state->head;

    while (curr && curr->next)
    {
        if (curr->isFree && curr->next->isFree &&
            ((i8 *)curr + curr->size == (i8 *)curr->next))
        {
            curr->size += curr->next->size;
            curr->next = curr->next->next;
        }
        else
        {
            curr = curr->next;
        }
    }
}

static void *__buffalloc_realloc(Allocator *this, void *ptr, u64 newSize)
{
    if (!ptr)
        return __buffalloc_alloc(this, newSize);

    if (newSize == 0)
    {
        __buffalloc_free(this, ptr);
        return NULL;
    }

    _BufferBlockHeader *header = (_BufferBlockHeader *)((i8 *)ptr - _X_BUFFALLOC_BLOCK_HEADER_SIZE);
    u64 usableSize = header->size - _X_BUFFALLOC_BLOCK_HEADER_SIZE;

    if (newSize <= usableSize)
        return ptr;

    void *newPtr = __buffalloc_alloc(this, newSize);
    if (!newPtr)
        return NULL;

    for (u64 i = 0; i < usableSize; ++i)
        ((i8 *)newPtr)[i] = ((i8 *)ptr)[i];

    __buffalloc_free(this, ptr);
    return newPtr;
}

/**
 * @brief Initializes a buffer allocator using the given byte buffer.
 *
 * Operates like the stdlib allocator, allows for freeing unlike the arena allocator.
 *
 * Memory passed must remain valid for the lifetime of the allocator.
 *
 * @param buffer Allocated buffer
 * @return ResultAllocator
 */
ResultAllocator buffer_allocator(Buffer buffer)
{
    if (!buffer.bytes || buffer.size < _X_BUFFALLOC_HEADER_SIZE + _X_BUFFALLOC_BLOCK_HEADER_SIZE)
        return (ResultAllocator){
            .value = {0},
            .error = ERR_INVALID_PARAMETER,
        };

    u64 alignedOffset = __buffalloc_offset_to_aligned((u64)buffer.bytes);
    u64 alignDiff = alignedOffset - (u64)buffer.bytes;
    u64 headerSize = sizeof(BufferAllocatorState);

    if (__buffalloc_offset_invalid(buffer.size, alignDiff, headerSize))
        return (ResultAllocator){
            .value = {0},
            .error = ERR_INVALID_PARAMETER,
        };

    BufferAllocatorState *state = (BufferAllocatorState *)(buffer.bytes + alignDiff);
    *state = (BufferAllocatorState){
        .buffer = buffer.bytes,
        .capacity = buffer.size,
        .head = NULL
    };

    u64 usableOffset = alignDiff + headerSize;
    u64 usableSize = buffer.size - usableOffset;

    _BufferBlockHeader *initial = (_BufferBlockHeader *)(buffer.bytes + usableOffset);
    *initial = (_BufferBlockHeader){
        .size = usableSize,
        .isFree = true,
        .next = NULL,
    };

    state->head = initial;

    return (ResultAllocator){
        .value = (Allocator){
            ._internalState = (void *)state,
            .alloc = __buffalloc_alloc,
            .realloc = __buffalloc_realloc,
            .free = __buffalloc_free,
        },
        .error = ERR_OK,
    };
}
