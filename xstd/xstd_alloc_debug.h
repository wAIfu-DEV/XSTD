#pragma once

#include "xstd_coretypes.h"
#include "xstd_alloc.h"
#include "xstd_io.h"

typedef struct _debug_allocator_state
{
    const Allocator *targetAllocator;
    u64 totalMallocCalls;
    u64 totalFreeCalls;
    u64 totalAllocBytes;
    u64 totalFreedBytes;
    u64 activeBytes;
    u64 activeAllocs;
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
 * @return Allocator
 */
Allocator debug_allocator(DebugAllocatorState *internalStatePtr, const Allocator *wrappedAllocator)
{
    *internalStatePtr = (DebugAllocatorState){
        .targetAllocator = wrappedAllocator,
        .totalMallocCalls = 0,
        .totalFreeCalls = 0,
        .totalAllocBytes = 0,
        .totalFreedBytes = 0,
        .activeAllocs = 0,
        .activeBytes = 0,
    };

    return (Allocator){
        ._internalState = internalStatePtr,
        .alloc = __debug_alloc,
        .realloc = __debug_realloc,
        .free = __debug_free,
    };
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

    // TODO: We need to implement a map structure to map pointers to their footprint
    // io_printerr("- Active bytes: ");
    // file_write_uint(&f, state->activeBytes);
    // io_printerrln("");
}
