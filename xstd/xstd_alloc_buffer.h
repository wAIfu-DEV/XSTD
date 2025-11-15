#pragma once

#include "xstd_core.h"
#include "xstd_alloc.h"
#include "xstd_error.h"
#include "xstd_mem.h"
#include "xstd_buffer.h"

typedef struct _buffalloc_block
{
    u64 size;                      // Total size including header
    struct _buffalloc_block *next; // Next block in the list
    ibool isFree;                  // If this block is free
} _BufferBlockHeader;

typedef struct _buffalloc_state
{
    i8 *buffer;
    u64 capacity;
    _BufferBlockHeader *head;
} BufferAllocatorState;

#define _X_BUFFALLOC_BLOCK_HEADER_SIZE ((u64)_buffalloc_offset_to_aligned(sizeof(_BufferBlockHeader)))

static inline u64 _buffalloc_offset_to_aligned(u64 offset)
{
    const u64 defaultAlign = 16;
    return (offset + defaultAlign - 1) & ~(defaultAlign - 1);
}

static inline ibool _buffalloc_offset_invalid(u64 totalCapacity, u64 alignedOffset, u64 allocSize)
{
    if ((alignedOffset + allocSize) > totalCapacity)
        return true;

    if (alignedOffset > ((u64)-1) - allocSize)
        return true;

    return false;
}

static void *_buffalloc_alloc(Allocator *a, u64 size)
{
    if (!a || !a->_internalState || size == 0)
        return NULL;

    BufferAllocatorState *state = (BufferAllocatorState *)a->_internalState;
    _BufferBlockHeader *curr = state->head;

    u64 totalSize = _buffalloc_offset_to_aligned(size) + _X_BUFFALLOC_BLOCK_HEADER_SIZE;

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

static void _buffalloc_free(Allocator *a, void *ptr)
{
    if (!a || !ptr)
        return;

    BufferAllocatorState *state = (BufferAllocatorState *)a->_internalState;
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

static void *_buffalloc_realloc(Allocator *a, void *ptr, u64 newSize)
{
    if (!ptr)
        return _buffalloc_alloc(a, newSize);

    if (newSize == 0)
    {
        _buffalloc_free(a, ptr);
        return NULL;
    }

    _BufferBlockHeader *header = (_BufferBlockHeader *)((i8 *)ptr - _X_BUFFALLOC_BLOCK_HEADER_SIZE);
    u64 usableSize = header->size - _X_BUFFALLOC_BLOCK_HEADER_SIZE;

    if (newSize <= usableSize)
        return ptr;

    void *newPtr = _buffalloc_alloc(a, newSize);
    if (!newPtr)
        return NULL;

    mem_copy(newPtr, ptr, usableSize);

    _buffalloc_free(a, ptr);
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
 * @exception ERR_INVALID_PARAMETER
 */
static inline ResultAllocator buffer_allocator(Buffer buffer)
{
    const u64 alignedHeaderSize = (u64)_buffalloc_offset_to_aligned(sizeof(BufferAllocatorState));

    if (!buffer.bytes || buffer.size < alignedHeaderSize + _X_BUFFALLOC_BLOCK_HEADER_SIZE)
        return (ResultAllocator){
            .error = X_ERR_EXT(
                "alloc_buffer", "buffer_allocator",
                ERR_INVALID_PARAMETER, "null or empty buff"),
        };

    u64 alignedOffset = _buffalloc_offset_to_aligned((u64)buffer.bytes);
    u64 alignDiff = alignedOffset - (u64)buffer.bytes;
    u64 headerSize = sizeof(BufferAllocatorState);

    if (_buffalloc_offset_invalid(buffer.size, alignDiff, headerSize))
        return (ResultAllocator){
            .error = X_ERR_EXT(
                "alloc_buffer", "buffer_allocator",
                ERR_INVALID_PARAMETER, "buffer too small for proper alignment"),
        };

    BufferAllocatorState *state = (BufferAllocatorState *)(buffer.bytes + alignDiff);
    *state = (BufferAllocatorState){
        .buffer = buffer.bytes,
        .capacity = buffer.size,
        .head = NULL};

    u64 usableOffset = alignDiff + headerSize;
    u64 usableSize = buffer.size - usableOffset;

    _BufferBlockHeader *initial = (_BufferBlockHeader *)(buffer.bytes + usableOffset);
    *initial = (_BufferBlockHeader){
        .size = usableSize,
        .next = NULL,
        .isFree = true,
    };

    state->head = initial;

    return (ResultAllocator){
        .value = (Allocator){
            ._internalState = (void *)state,
            .alloc = _buffalloc_alloc,
            .realloc = _buffalloc_realloc,
            .free = _buffalloc_free,
        },
        .error = X_ERR_OK,
    };
}
