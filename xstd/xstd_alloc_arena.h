#pragma once

#include "xstd_core.h"
#include "xstd_buffer.h"
#include "xstd_alloc.h"

#define _X_ARENA_ALLOC_DEFAULT_ALIGN 16

// ArenaAllocator State
typedef struct _arena_allocator_state
{
    i8 *buffer;        // start of buffer
    u64 capacity;      // total size
    u64 headerSize;    // size of aligned state
    u64 offset;        // bytes used
    ibool bufferOwned; // if true, buffer is heap-allocated and should be freed
} ArenaAllocatorState;

u64 __arena_offset_to_aligned(u64 offset)
{
    return (offset + _X_ARENA_ALLOC_DEFAULT_ALIGN - 1) & ~(_X_ARENA_ALLOC_DEFAULT_ALIGN - 1);
}

ibool __arena_offset_invalid(u64 totalCapacity, u64 alignedOffset, u64 allocSize)
{
    if ((alignedOffset + allocSize) > totalCapacity)
        return true;

    if (alignedOffset > ((u64)-1) - allocSize)
        return true;

    return false;
}

void *__arena_alloc(Allocator *this, u64 size)
{
    if (!this || !this->_internalState)
        return NULL;

    ArenaAllocatorState *state = (ArenaAllocatorState *)this->_internalState;
    if (!state->buffer || size == 0)
        return NULL;

    u64 alignedOffset = __arena_offset_to_aligned(state->offset);

    if (__arena_offset_invalid(state->capacity, alignedOffset, size))
        return NULL;

    void *out = state->buffer + alignedOffset;
    state->offset = alignedOffset + size;
    return out;
}

void *__arena_realloc(Allocator *this, void *block, u64 newSize)
{
    if (!this || !this->_internalState)
        return NULL;

    ArenaAllocatorState *state = (ArenaAllocatorState *)this->_internalState;
    if (!state->buffer || newSize == 0 || !block)
        return NULL;

    i8 *blockPtr = (i8 *)block;
    u64 blockOffset = blockPtr - state->buffer;

    // If block is at end
    if (blockOffset == (__arena_offset_to_aligned(state->offset) - newSize))
    {
        u64 alignedOffset = __arena_offset_to_aligned(blockOffset);

        if (__arena_offset_invalid(state->capacity, alignedOffset, newSize))
            return NULL;

        state->offset = alignedOffset + newSize;
        return block;
    }

    return __arena_alloc(this, newSize);
}

void __arena_free(Allocator *this, void *block)
{
    (void)this;
    (void)block;
}

/**
 * @brief Clears the contents of the of the arena, allows for reuse.
 *
 * IMPORTANT: Make sure no dangling pointers are pointing to arena memory as those
 * pointers will become invalid and cause undefined behavior.
 *
 * @param arena
 */
void arena_allocator_clear(Allocator *arena)
{
    ArenaAllocatorState *state = (ArenaAllocatorState *)arena->_internalState;
    state->offset = state->headerSize;
}

ArenaAllocatorState *__arena_alloc_header(Buffer buff, ibool isHeap)
{
    u64 headerSize = sizeof(ArenaAllocatorState);
    u64 alignedOffset = __arena_offset_to_aligned((u64)buff.bytes);
    u64 alignDiff = alignedOffset - (u64)buff.bytes;

    if (__arena_offset_invalid(buff.size, alignDiff, headerSize))
        return NULL;

    ArenaAllocatorState *state = (ArenaAllocatorState *)(buff.bytes + alignDiff);

    *state = (ArenaAllocatorState){
        .buffer = buff.bytes,
        .capacity = buff.size,
        .headerSize = alignDiff + headerSize,
        .offset = alignDiff + headerSize,
        .bufferOwned = isHeap,
    };
    return state;
}

/**
 * @brief Create an arena allocator using a stack or heap allocated buffer.
 * If memory is heap allocated, set isHeap to true.
 *
 * ```c
 * u8 buffer[4096]; ArenaAllocatorState state;
 * Buffer arenaBuff = (Buffer){ .bytes = buffer, size = 4096 };
 * Allocator arena = arena_allocator_init(buffer, false);
 * ```
 *
 * If you wish to free the memory allocated for the arena:
 *
 * - Stack allocated: let the buffer fall out of scope (i.e. do nothing)
 *
 * - Heap allocated: free the memory allocated for the buffer used in call to `arena_allocator_init()`
 *
 * @param buffer buffer pointing to allocated memory
 * @param isHeap true if is heap allocated, false if not
 * @return Allocator
 * @exception ERR_INVALID_PARAMETER, ERR_OUT_OF_MEMORY
 */
ResultAllocator arena_allocator(Buffer buffer, ibool isHeap)
{
    if (!buffer.bytes || buffer.size == 0)
        return (ResultAllocator){
            .value = {0},
            .error = X_ERR_EXT("alloc_arena", "arena_allocator", ERR_INVALID_PARAMETER, "null or empty buff"),
        };

    ArenaAllocatorState *state = __arena_alloc_header(buffer, isHeap);
    if (!state)
        return (ResultAllocator){
            .value = {0},
            .error = X_ERR_EXT("alloc_arena", "arena_allocator", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    return (ResultAllocator){
        .value = (Allocator){
            ._internalState = state,
            .alloc = __arena_alloc,
            .realloc = __arena_realloc,
            .free = __arena_free,
        },
        .error = X_ERR_OK,
    };
}
