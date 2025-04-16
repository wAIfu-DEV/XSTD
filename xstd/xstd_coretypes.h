#pragma once

typedef char ibool; // 8bit value representing two states: 0 (false) | not 0 (true)

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

typedef char i8;       // 8bit value representing an integer number or character.
typedef short i16;     // 16bit value representing an integer number.
typedef int i32;       // 32bit value representing an integer number.
typedef long long i64; // 64bit value representing an integer number.

typedef unsigned char u8;       // 8bit value representing an unsigned integer number.
typedef unsigned short u16;     // 16bit value representing an unsigned integer number.
typedef unsigned int u32;       // 32bit value representing an unsigned integer number.
typedef unsigned long long u64; // 64bit value representing an unsigned integer number.

typedef float f32;  // 32bit value representing a decimal number.
typedef double f64; // 64bit value representing a decimal number.

typedef i8 *String;         // Pointer to string of characters terminated with 0.
typedef const i8 *ConstStr; // Pointer to immutable string of characters terminated with 0.
typedef i8 *HeapStr;        // Pointer to heap-allocated string of characters terminated with 0. Should be freed.

// Represents an area of memory either on the stack or on the heap.
typedef struct _buffer
{
    i8 *bytes;
    u64 size;
} Buffer;

typedef Buffer HeapBuff; // Represents an area of memory on the heap.

// Represents an area of immutable memory either on the stack or on the heap.
typedef struct _buffer_const
{
    const i8 *bytes;
    u64 size;
} ConstBuff;
