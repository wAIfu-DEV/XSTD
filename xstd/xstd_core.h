#pragma once

#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(__ppc64__)
    #define __XSTD_ARCH_64BIT 1
#else
    #define __XSTD_ARCH_64BIT 0
#endif


#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef NULL
#define NULL 0
#endif

typedef char ibool; // 8bit value representing two states: 0 (false) | not 0 (true)

typedef char i8;       // 8bit value representing an integer number or character.
typedef short i16;     // 16bit value representing an integer number.
typedef int i32;       // 32bit value representing an integer number.

typedef unsigned char u8;       // 8bit value representing an unsigned integer number.
typedef unsigned short u16;     // 16bit value representing an unsigned integer number.
typedef unsigned int u32;       // 32bit value representing an unsigned integer number.

#if __XSTD_ARCH_64BIT
typedef long long i64; // 64bit value representing an integer number.
typedef unsigned long long u64; // 64bit value representing an unsigned integer number.
#else
typedef i32 i64; // 64bit value representing an integer number.
typedef u32 u64; // 64bit value representing an unsigned integer number.
#endif

typedef float f32;  // 32bit value representing a decimal number.

#if __XSTD_ARCH_64BIT
typedef double f64; // 64bit value representing a decimal number.
#else
typedef f32 f64; // 64bit value representing a decimal number.
#endif

#define U8_MAXVAL (u8)-1
#define U16_MAXVAL (u16)-1
#define U32_MAXVAL (u32)-1
#define U64_MAXVAL (u64)-1

#define I8_MAXVAL (i8)127
#define I8_MINVAL (i8)(-128)
#define I16_MAXVAL (i16)32767
#define I16_MINVAL (i16)(-32768)
#define I32_MAXVAL (i32)2147483647
#define I32_MINVAL (i32)(-2147483648)

#if __XSTD_ARCH_64BIT
#define I64_MAXVAL (i64)((u64)-1 / (i64)2)
#define I64_MINVAL (i64)(-((u64)-1 / (i64)2) - (i64)1)
#else
#define I64_MAXVAL I32_MAXVAL
#define I64_MINVAL I32_MINVAL
#endif

typedef i8 *String;         // Pointer to string of characters terminated with 0.
typedef const i8 *ConstStr; // Pointer to immutable string of characters terminated with 0.
typedef i8 *HeapStr;        // Pointer to heap-allocated string of characters terminated with 0.
typedef i8 *OwnedStr;       // Pointer to heap-allocated string of characters terminated with 0. Should be freed.

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


