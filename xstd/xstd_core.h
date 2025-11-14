#pragma once

#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(__ppc64__)
    #define _XSTD_ARCH_64BIT 1
#else
    #define _XSTD_ARCH_64BIT 0
#endif

#ifndef __cplusplus
    #ifndef true
        #define true 1
    #endif

    #ifndef false
        #define false 0
    #endif
#endif

#ifndef NULL
    #define NULL 0
#endif

#if _WIN32 // || __MINGW32__ || __CYGWIN__
    #define _X_PLAT_WIN 1
#endif

typedef char ibool; // 8bit value representing two states: 0 (false) | not 0 (true)

typedef char i8;       // 8bit value representing an integer number or character.
typedef short i16;     // 16bit value representing an integer number.
typedef int i32;       // 32bit value representing an integer number.

typedef unsigned char u8;       // 8bit value representing an unsigned integer number.
typedef unsigned short u16;     // 16bit value representing an unsigned integer number.
typedef unsigned int u32;       // 32bit value representing an unsigned integer number.

//#if _XSTD_ARCH_64BIT
    typedef long long i64; // 64bit value representing an integer number.
    typedef unsigned long long u64; // 64bit value representing an unsigned integer number.
    typedef unsigned long long uPtr; // 64bit value representing a memory address of a pointer.
/*#else
    typedef i32 i64; // 32bit value representing an integer number.
    typedef u32 u64; // 32bit value representing an unsigned integer number.
    typedef u32 uPtr; // 32bit value representing a memory address of a pointer.
#endif*/

typedef float f32;  // 32bit value representing a decimal number.
typedef double f64; // 64bit value representing a decimal number.

/*
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

#if _XSTD_ARCH_64BIT
    #define I64_MAXVAL (i64)((u64)-1 / (i64)2)
    #define I64_MINVAL (i64)(-((u64)-1 / (i64)2) - (i64)1)
#else
    #define I64_MAXVAL I32_MAXVAL
    #define I64_MINVAL I32_MINVAL
#endif*/

static const struct {
    const u8 U8;
    const i8 I8_MIN;
    const i8 I8_MAX;
    const u16 U16;
    const i16 I16_MIN;
    const i16 I16_MAX;
    const u32 U32;
    const i32 I32_MIN;
    const i32 I32_MAX;
    const u64 U64;
    const i64 I64_MIN;
    const i64 I64_MAX;
} MaxVals = {
    .U8 = (u8)-1,
    .U16 = (u16)-1,
    .U32 = (u32)-1,
    .U64 = (u64)-1,
    
    .I8_MAX = (i8)127,
    .I8_MIN = (i8)(-128),
    .I16_MAX = (i16)32767,
    .I16_MIN = (i16)(-32768),
    .I32_MAX = (i32)2147483647,
    .I32_MIN = (i32)(-2147483648),
    .I64_MAX = (i64)((u64)-1 / (i64)2),
    .I64_MIN = (i64)(-((u64)-1 / (i64)2) - (i64)1),
};

// ASCII/UTF8 strings
typedef i8 *String;         // Pointer to string of characters terminated with 0.
typedef const i8 *ConstStr; // Pointer to immutable string of characters terminated with 0.
typedef i8 *HeapStr;        // Pointer to heap-allocated string of characters terminated with 0.
typedef i8 *OwnedStr;       // Pointer to heap-allocated string of characters terminated with 0. Should be freed.

// Widechar UTF16 strings
typedef i16* Utf16Str;            // Pointer to UTF-16 string terminated with 0.
typedef const i16* Utf16ConstStr; // Pointer to immutable string of 16 bit characters terminated with 0.
typedef i16* Utf16HeapStr;        // Pointer to heap-allocated string of 16 bit characters terminated with 0.
typedef i16* Utf16OwnedStr;       // Pointer to heap-allocated UTF-16 string terminated with 0. Should be freed.

typedef struct _utf16_buff
{
    Utf16Str units; // Pointer to valid UTF-16 code units.
    u64 size;            // Number of UTF-16 code units available from units.
} Utf16Buff;

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
