#pragma once

#include "xstd_core.h"
#include "xstd_alloc.h"

#define _X_DEBUG_ALLOC_ENTRY_EMPTY 0u
#define _X_DEBUG_ALLOC_ENTRY_FULL 1u
#define _X_DEBUG_ALLOC_ENTRY_TOMB 2u

typedef struct _debug_allocator_entry
{
    void *ptr;
    u64 size;
    u8 state;
} DebugAllocEntry;

typedef struct _debug_allocator_state
{
    Allocator *targetAllocator;
    DebugAllocEntry *table;
    u32 capacity;
    u32 count;
    u32 mask;
    u32 resizeThreshold;

    u64 activeAllocCount;
    u64 peakAllocCount;

    u64 activeUserBytes;
    u64 peakUserBytes;

    u64 activeOverheadBytes;
    u64 peakOverheadBytes;

    u64 totalMallocCalls;
    u64 totalFreeCalls;
    u64 totalAllocBytes;
    u64 totalFreedBytes;

    u64 totalMetaAllocBytes;
    u64 totalMetaFreeBytes;

    ibool trackingOverflow;
    u64 failedInsertions;
    u64 untrackedFrees;
    u64 untrackedReallocs;
} DebugAllocatorState;

static inline u32 _debug_allocator_hash_ptr(void *ptr)
{
#if _X_ARCH_64BIT
    u64 key = (u64)(unsigned long long)ptr;
#else
    u64 key = (u64)(unsigned long)ptr;
#endif
    key ^= key >> 17;
    key ^= key >> 11;
    key *= 0x9e3779b1u;
    return (u32)key;
}

static inline u32 _debug_allocator_next_pow2(u32 value)
{
    if (value < 4u)
        return 4u;

    value -= 1u;
    value |= value >> 1u;
    value |= value >> 2u;
    value |= value >> 4u;
    value |= value >> 8u;
    value |= value >> 16u;
    return value + 1u;
}

static inline u32 _debug_allocator_resize_threshold(u32 capacity)
{
    u32 threshold = (capacity * 3u) / 4u;
    if (threshold >= capacity)
        threshold = capacity - 1u;
    if (threshold == 0u)
        threshold = capacity > 1u ? capacity - 1u : 1u;
    return threshold;
}

static inline DebugAllocEntry *_debug_allocator_table_alloc(DebugAllocatorState *state, u32 capacity)
{
    if (!state || !state->targetAllocator)
        return NULL;

    u64 bytes = (u64)capacity * (u64)sizeof(DebugAllocEntry);
    DebugAllocEntry *entries = (DebugAllocEntry *)state->targetAllocator->alloc(state->targetAllocator, bytes);
    if (!entries)
        return NULL;

    for (u32 i = 0; i < capacity; ++i)
    {
        entries[i].ptr = NULL;
        entries[i].size = 0;
        entries[i].state = _X_DEBUG_ALLOC_ENTRY_EMPTY;
    }

    state->totalMetaAllocBytes += bytes;
    state->activeOverheadBytes += bytes;
    if (state->activeOverheadBytes > state->peakOverheadBytes)
        state->peakOverheadBytes = state->activeOverheadBytes;

    return entries;
}

static inline void _debug_allocator_table_free(DebugAllocatorState *state, DebugAllocEntry *table, u32 capacity)
{
    if (!state || !state->targetAllocator || !table)
        return;

    u64 bytes = (u64)capacity * (u64)sizeof(DebugAllocEntry);
    if (state->activeOverheadBytes >= bytes)
        state->activeOverheadBytes -= bytes;
    else
        state->activeOverheadBytes = 0;

    state->totalMetaFreeBytes += bytes;
    state->targetAllocator->free(state->targetAllocator, table);
}

static inline DebugAllocEntry *_debug_allocator_find(DebugAllocatorState *state, void *ptr, u32 *indexOut)
{
    if (!state || !state->table || state->capacity == 0u)
        return NULL;

    u32 mask = state->mask;
    u32 idx = _debug_allocator_hash_ptr(ptr) & mask;
    DebugAllocEntry *table = state->table;

    while (1)
    {
        DebugAllocEntry *entry = &table[idx];
        if (entry->state == _X_DEBUG_ALLOC_ENTRY_EMPTY)
            break;

        if (entry->state == _X_DEBUG_ALLOC_ENTRY_FULL && entry->ptr == ptr)
        {
            if (indexOut)
                *indexOut = idx;
            return entry;
        }

        idx = (idx + 1u) & mask;
    }

    if (indexOut)
        *indexOut = idx;

    return NULL;
}

static inline ibool _debug_allocator_grow(DebugAllocatorState *state);

static inline DebugAllocEntry *_debug_allocator_insert(DebugAllocatorState *state, void *ptr, u64 size, ibool isReinsert)
{
    if (!state || !state->table)
        return NULL;

    if (!isReinsert && state->count + 1u > state->resizeThreshold)
    {
        if (!_debug_allocator_grow(state))
            return NULL;
    }

    DebugAllocEntry *table = state->table;
    u32 mask = state->mask;
    u32 idx = _debug_allocator_hash_ptr(ptr) & mask;
    u32 firstTombstone = 0xFFFFFFFFu;

    while (1)
    {
        DebugAllocEntry *entry = &table[idx];
        if (entry->state == _X_DEBUG_ALLOC_ENTRY_FULL)
        {
            if (entry->ptr == ptr)
            {
                if (!isReinsert)
                {
                    if (state->activeUserBytes >= entry->size)
                        state->activeUserBytes -= entry->size;
                    else
                        state->activeUserBytes = 0;

                    state->activeUserBytes += size;
                    if (state->activeUserBytes > state->peakUserBytes)
                        state->peakUserBytes = state->activeUserBytes;
                }

                entry->size = size;
                entry->ptr = ptr;
                return entry;
            }
        }
        else if (entry->state == _X_DEBUG_ALLOC_ENTRY_EMPTY)
        {
            u32 targetIdx = (firstTombstone != 0xFFFFFFFFu) ? firstTombstone : idx;
            DebugAllocEntry *target = &table[targetIdx];
            target->ptr = ptr;
            target->size = size;
            target->state = _X_DEBUG_ALLOC_ENTRY_FULL;

            state->count += 1u;
            if (!isReinsert)
            {
                state->activeUserBytes += size;
                if (state->activeUserBytes > state->peakUserBytes)
                    state->peakUserBytes = state->activeUserBytes;
            }
            return target;
        }
        else
        {
            if (firstTombstone == 0xFFFFFFFFu)
                firstTombstone = idx;
        }

        idx = (idx + 1u) & mask;
    }
}

static inline void _debug_allocator_remove(DebugAllocatorState *state, u32 index)
{
    if (!state || !state->table || index >= state->capacity)
        return;

    DebugAllocEntry *entry = &state->table[index];
    if (entry->state != _X_DEBUG_ALLOC_ENTRY_FULL)
        return;

    entry->state = _X_DEBUG_ALLOC_ENTRY_TOMB;
    entry->ptr = NULL;
    entry->size = 0;

    if (state->count > 0u)
        state->count -= 1u;
}

static inline ibool _debug_allocator_grow(DebugAllocatorState *state)
{
    if (!state || !state->table)
        return false;

    u32 oldCapacity = state->capacity;
    if (oldCapacity >= (1u << 30))
        return false;

    u32 newCapacity = oldCapacity << 1;
    DebugAllocEntry *newTable = _debug_allocator_table_alloc(state, newCapacity);
    if (!newTable)
        return false;

    DebugAllocEntry *oldTable = state->table;

    state->table = newTable;
    state->capacity = newCapacity;
    state->mask = newCapacity - 1u;
    state->resizeThreshold = _debug_allocator_resize_threshold(newCapacity);

    state->count = 0u;

    for (u32 i = 0u; i < oldCapacity; ++i)
    {
        if (oldTable[i].state == _X_DEBUG_ALLOC_ENTRY_FULL)
            _debug_allocator_insert(state, oldTable[i].ptr, oldTable[i].size, true);
    }

    _debug_allocator_table_free(state, oldTable, oldCapacity);
    return true;
}

static void *_debug_alloc(Allocator *a, u64 size)
{
    if (!a || size == 0u)
        return NULL;

    DebugAllocatorState *state = (DebugAllocatorState *)a->_internalState;
    if (!state || !state->targetAllocator)
        return NULL;

    void *ptr = state->targetAllocator->alloc(state->targetAllocator, size);
    if (!ptr)
        return NULL;

    DebugAllocEntry *entry = _debug_allocator_insert(state, ptr, size, false);
    if (!entry)
    {
        state->trackingOverflow = true;
        state->failedInsertions += 1u;
        state->targetAllocator->free(state->targetAllocator, ptr);
        return NULL;
    }

    state->activeAllocCount += 1u;
    if (state->activeAllocCount > state->peakAllocCount)
        state->peakAllocCount = state->activeAllocCount;

    state->totalMallocCalls += 1u;
    state->totalAllocBytes += size;
    return ptr;
}

static void _debug_free(Allocator *a, void *block)
{
    if (!a || !block)
        return;

    DebugAllocatorState *state = (DebugAllocatorState *)a->_internalState;
    if (!state || !state->targetAllocator)
        return;

    u32 index = 0u;
    DebugAllocEntry *entry = _debug_allocator_find(state, block, &index);

    state->targetAllocator->free(state->targetAllocator, block);
    state->totalFreeCalls += 1u;

    if (!entry)
    {
        state->untrackedFrees += 1u;
        return;
    }

    if (state->activeUserBytes >= entry->size)
        state->activeUserBytes -= entry->size;
    else
        state->activeUserBytes = 0;

    if (state->activeAllocCount > 0u)
        state->activeAllocCount -= 1u;

    state->totalFreedBytes += entry->size;
    _debug_allocator_remove(state, index);
}

static void *_debug_realloc(Allocator *a, void *block, u64 newSize)
{
    if (!a)
        return NULL;

    DebugAllocatorState *state = (DebugAllocatorState *)a->_internalState;
    if (!state || !state->targetAllocator)
        return NULL;

    if (!block)
        return _debug_alloc(a, newSize);

    if (newSize == 0u)
    {
        _debug_free(a, block);
        return NULL;
    }

    u32 oldIndex = 0u;
    DebugAllocEntry *entry = _debug_allocator_find(state, block, &oldIndex);
    u64 oldSize = entry ? entry->size : 0u;

    void *ptr = state->targetAllocator->realloc(state->targetAllocator, block, newSize);
    if (!ptr)
        return NULL;

    state->totalMallocCalls += 1u;
    state->totalAllocBytes += newSize;

    if (ptr == block)
    {
        DebugAllocEntry *updated = _debug_allocator_insert(state, ptr, newSize, false);
        if (!updated)
        {
            state->trackingOverflow = true;
            state->failedInsertions += 1u;
        }
        return ptr;
    }

    if (entry)
    {
        if (state->activeUserBytes >= oldSize)
            state->activeUserBytes -= oldSize;
        else
            state->activeUserBytes = 0;

        if (state->activeAllocCount > 0u)
            state->activeAllocCount -= 1u;

        state->totalFreedBytes += oldSize;
        _debug_allocator_remove(state, oldIndex);
    }
    else
    {
        state->untrackedReallocs += 1u;
    }

    DebugAllocEntry *newEntry = _debug_allocator_insert(state, ptr, newSize, false);
    if (!newEntry)
    {
        state->trackingOverflow = true;
        state->failedInsertions += 1u;
        state->targetAllocator->free(state->targetAllocator, ptr);
        return NULL;
    }

    state->activeAllocCount += 1u;
    if (state->activeAllocCount > state->peakAllocCount)
        state->peakAllocCount = state->activeAllocCount;

    return ptr;
}

/**
 * @brief Creates a debug allocator wrapping another allocator that tracks allocations.
 *
 * ```c
 * DebugAllocatorState state;
 * ResultAllocator dbgAllocRes = debug_allocator(&state, 128, default_allocator());
 * if (dbgAllocRes.error.code) // Error!
 * // Do stuff with allocator
 *
 * // Print debug allocation stats
 * if (state.activeAllocCount > 0) {
 *  io_print("Leaky allocations: ");
 *  io_print_uint(state.activeAllocCount);
 *  io_print("\n");
 *  io_print("Leaked bytes: ");
 *  io_print_uint(state.activeUserBytes);
 *  io_print("\n");
 * }
 * ```
 *
 * @param state
 * @param requestedCapacity
 * @param wrappedAllocator
 * @return ResultAllocator
 * @exception ERR_INVALID_PARAMETER, ERR_OUT_OF_MEMORY
 */
static inline ResultAllocator debug_allocator(DebugAllocatorState *state, u32 requestedCapacity, Allocator *wrappedAllocator)
{
    if (!state || !wrappedAllocator)
        return (ResultAllocator){
            .error = X_ERR_EXT("alloc_debug", "debug_allocator", ERR_INVALID_PARAMETER, "null arg"),
        };

    u32 capacity = _debug_allocator_next_pow2(requestedCapacity);
    if (capacity < 4u)
        capacity = 4u;

    *state = (DebugAllocatorState){
        .targetAllocator = wrappedAllocator,
        .table = NULL,
        .capacity = capacity,
        .count = 0,
        .mask = capacity - 1,
        .resizeThreshold = _debug_allocator_resize_threshold(capacity),
        .activeAllocCount = 0,
        .peakAllocCount = 0,
        .activeUserBytes = 0,
        .peakUserBytes = 0,
        .activeOverheadBytes = 0,
        .peakOverheadBytes = 0,
        .totalMallocCalls = 0,
        .totalFreeCalls = 0,
        .totalAllocBytes = 0,
        .totalFreedBytes = 0,
        .totalMetaAllocBytes = 0,
        .totalMetaFreeBytes = 0,
        .trackingOverflow = false,
        .failedInsertions = 0,
        .untrackedFrees = 0,
        .untrackedReallocs = 0,
    };

    DebugAllocEntry *table = _debug_allocator_table_alloc(state, capacity);
    if (!table)
        return (ResultAllocator){
            .error = X_ERR_EXT(
                "alloc_debug", "debug_allocator",
                ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    state->table = table;

    return (ResultAllocator){
        .value = (Allocator){
            ._internalState = state,
            .alloc = _debug_alloc,
            .realloc = _debug_realloc,
            .free = _debug_free,
        },
        .error = X_ERR_OK,
    };
}
