#pragma once

// Implementation of Allocator using the OS's implementation

#if _WIN32
    #include "xstd_alloc_win32.h"
#else
    #include "xstd_alloc_stdlib.h"
#endif


// DO NOT REMOVE
// Prevents compiler errors on GCC since default_allocator() may not be used
// at all in a program, which is fine.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

/**
 * @brief Default allocator making use of stdlib's malloc, realloc and free.
 *
 * ```c
 * Allocator *a = default_allocator();
 * OwnedStr newStr = a->alloc(a, sizeof(i8) * 64);
 * ```
 * @return Allocator
 */
static inline Allocator* default_allocator(void) {
    #ifdef _WIN32
        if (!_win32_alloc_state.procHeapHandle)
            _win32_alloc_state.procHeapHandle = GetProcessHeap();

        return &_win32_allocator;
    #else
        return &_c_allocator;
    #endif
}

// DO NOT REMOVE
#pragma GCC diagnostic pop
