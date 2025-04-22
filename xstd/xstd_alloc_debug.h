#pragma once

#include "xstd_core.h"
#include "xstd_alloc.h"
#include "xstd_io.h"
#include "xstd_hashmap.h"

typedef struct _debug_allocator_state
{
    Allocator *targetAllocator;
    u64 totalMallocCalls;
    u64 totalFreeCalls;
    u64 totalAllocBytes;
    u64 totalFreedBytes;
    u64 activeBytes;
    u64 activeAllocs;

    HashMap ptrAllocMap;

    ibool verbosePrint;
} DebugAllocatorState;

void *__debug_alloc(Allocator *this, u64 size)
{
    DebugAllocatorState *state = (DebugAllocatorState *)this->_internalState;

    void *ptr = state->targetAllocator->alloc((Allocator *)state->targetAllocator, size);

    if (!ptr)
    {
        if (state->verbosePrint)
            io_printerrln("[DEBUGALLOC]: Allocation failure.");
        return NULL;
    }

    state->totalMallocCalls += 1;
    state->totalAllocBytes += size;
    state->activeAllocs += 1;
    state->activeBytes += size;

    // ptrAllocMap[ptr] = size
    Buffer mapKey = (Buffer){ .bytes = (i8*)&ptr, .size = sizeof(u64) };
    Error _ = hashmap_set(&state->ptrAllocMap, mapKey, &size);
    (void)_;

    if (state->verbosePrint)
    {
        File f = IoStderr;
        io_printerr("[DEBUGALLOC]: Allocated ");
        file_write_uint(&f, size);
        io_printerr(" bytes @ ");
        file_write_uint(&f, (u64)ptr);
        io_printerrln("");
    }

    return ptr;
}

void *__debug_realloc(Allocator *this, void *block, u64 newSize)
{
    DebugAllocatorState *state = (DebugAllocatorState *)this->_internalState;

    void *ptr = state->targetAllocator->realloc((Allocator *)state->targetAllocator, block, newSize);

    if (!ptr)
    {
        if (state->verbosePrint)
            io_printerrln("[DEBUGALLOC]: Reallocation failure.");
        return NULL;
    }

    state->totalMallocCalls += 1;
    state->totalAllocBytes += newSize;
    state->activeAllocs += 1;
    state->activeBytes += newSize;

    // ptrAllocMap[ptr] = size
    Buffer mapKey = (Buffer){ .bytes = (i8*)&ptr, .size = sizeof(u64) };
    Error _ = hashmap_set(&state->ptrAllocMap, mapKey, &newSize);
    (void)_;

    if (state->verbosePrint)
    {
        File f = IoStderr;
        io_printerr("[DEBUGALLOC]: Reallocated to ");
        file_write_uint(&f, newSize);
        io_printerr(" bytes @ ");
        file_write_uint(&f, (u64)ptr);
        io_printerrln("");
    }

    return ptr;
}

void __debug_free(Allocator *this, void *block)
{
    DebugAllocatorState *state = (DebugAllocatorState *)this->_internalState;

    if (state->verbosePrint)
    {
        File f = IoStderr;
        io_printerr("[DEBUGALLOC]: Freed block @ ");
        file_write_uint(&f, (u64)block);
        io_printerrln("");
    }

    state->totalFreeCalls += 1;

    if (state->activeAllocs > 0)
        state->activeAllocs -= 1;
    
    Buffer mapKey = (Buffer){ .bytes = (i8*)&block, .size = sizeof(u64) };
    Error _ = hashmap_remove(&state->ptrAllocMap, mapKey);
    (void)_;

    state->targetAllocator->free((Allocator *)state->targetAllocator, block);
}

/**
 * @brief Returns a debug allocator that wraps the provided `wrappedAllocator`.
 * The allocator will log all allocations/frees and track memory usage.
 *
 * You must retain access to the internalStatePtr as long as the allocator is being used.
 *
 * ```c
 * DebugAllocatorState dbgState;
 * Allocator dbgAlloc = debug_allocator(&dbgState, &c_allocator);
 * ```
 * @param internalStatePtr pointer to a persistent DebugAllocatorState
 * @param wrappedAllocator allocator to forward calls to
 * @return ResultAllocator
 */
ResultAllocator debug_allocator(DebugAllocatorState *internalStatePtr, Allocator *wrappedAllocator)
{
    if (!wrappedAllocator || !internalStatePtr)
        return (ResultAllocator){
            .value = {0},
            .error = ERR_INVALID_PARAMETER,
        };

    *internalStatePtr = (DebugAllocatorState){
        .targetAllocator = wrappedAllocator,
        .totalMallocCalls = 0,
        .totalFreeCalls = 0,
        .totalAllocBytes = 0,
        .totalFreedBytes = 0,
        .activeAllocs = 0,
        .activeBytes = 0,
    };

    ResultHashMap hmRes = HashMapInitT(u64, wrappedAllocator);

    if (hmRes.error)
        return (ResultAllocator){
            .value = {0},
            .error = hmRes.error,
        };
    
    internalStatePtr->ptrAllocMap = hmRes.value;

    return (ResultAllocator){
        .value = (Allocator) {
            ._internalState = internalStatePtr,
            .alloc = __debug_alloc,
            .realloc = __debug_realloc,
            .free = __debug_free,
        },
        .error = ERR_OK,
    };
}

void __debug_allocator_print_active(Buffer key, void* value, void* userArg)
{
    File f = IoStderr;

    file_write_str(&f, "ptr=");
    file_write_uint(&f, *(u64*)key.bytes);
    file_write_str(&f, " size=");
    file_write_uint(&f, *(u64*)value);
    file_write_char(&f, '\n');

    file_flush(&f);
}

/**
 * @brief Logs debug allocator statistics to stderr
 */
void debug_allocator_logstats(DebugAllocatorState *state)
{
    File f = IoStderr;
    io_printerrln("[DEBUGALLOC STATS]:");
    io_printerr("- Total allocs: ");
    file_write_uint(&f, state->totalMallocCalls);
    io_printerrln("");

    io_printerr("- Total frees: ");
    file_write_uint(&f, state->totalFreeCalls);
    io_printerrln("");

    io_printerr("- Total bytes allocated: ");
    file_write_uint(&f, state->totalAllocBytes);
    io_printerrln("");

    io_printerr("- Active allocs: ");
    file_write_uint(&f, state->activeAllocs);
    io_printerrln("");

    io_printerr("- Active bytes:\n");
    hashmap_for_each(&state->ptrAllocMap, __debug_allocator_print_active, NULL);

    if (state->activeAllocs == 0)
    {
        io_printerrln("none");
    }
}
