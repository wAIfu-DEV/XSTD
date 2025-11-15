#pragma once

#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__) || defined(__ppc64__)
    #define _X_ARCH_64BIT 1
#else
    #define _X_ARCH_64BIT 0
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




// Equivalent to stdint.h making use of compiler-defined macros for int width
// correctness.
// Largely inspired by llvm's stdint implementation.
// https://github.com/llvm-mirror/clang/blob/master/lib/Headers/stdint.h

typedef __INT8_TYPE__ ibool; // 8bit value representing two states: 0 (false) | not 0 (true)

typedef __WCHAR_TYPE__ wChar; // 16bit wide chars for UTF-16 or other encodings.

typedef __INT8_TYPE__ i8;         // 8bit value representing an integer number or character.
typedef __INT16_TYPE__ i16;       // 16bit value representing an integer number.
typedef __INT32_TYPE__ i32;       // 32bit value representing an integer number.

typedef __UINT8_TYPE__ u8;        // 8bit value representing an unsigned integer number.
typedef __UINT16_TYPE__ u16;      // 16bit value representing an unsigned integer number.
typedef __UINT32_TYPE__ u32;      // 32bit value representing an unsigned integer number.

typedef __INT64_TYPE__ i64;       // 64bit value representing an integer number.
typedef __UINT64_TYPE__ u64;      // 64bit value representing an unsigned integer number.

typedef __INTPTR_TYPE__ iPtr;     // signed 64bit value representing a memory address of a pointer.
typedef unsigned __INTPTR_TYPE__ uPtr; // unsigned 64bit value representing a memory address of a pointer.

typedef float f32;  // 32bit value representing a decimal number.
typedef double f64; // 64bit value representing a decimal number.

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
} EnumMaxVal = {
    .U8 = (u8)__UINT8_MAX__,
    .U16 = (u16)__UINT16_MAX__,
    .U32 = (u32)__UINT32_MAX__,
    .U64 = (u64)__UINT64_MAX__,
    
    .I8_MAX = (i8)__INT8_MAX__,
    .I8_MIN = (i8)(-(__INT8_MAX__)) - 1,
    .I16_MAX = (i16)__INT16_MAX__,
    .I16_MIN = (i16)(-(__INT16_MAX__)) - 1,
    .I32_MAX = (i32)__INT32_MAX__,
    .I32_MIN = (i32)(-(__INT32_MAX__)) - 1,
    .I64_MAX = (i64)__INT64_MAX__,
    .I64_MIN = (i64)(-(__INT64_MAX__)) - 1,
};

// ASCII/UTF8 strings
typedef char *String;         // Pointer to string of characters terminated with 0.
typedef const char *ConstStr; // Pointer to immutable string of characters terminated with 0.
typedef char *HeapStr;        // Pointer to heap-allocated string of characters terminated with 0.
typedef char *OwnedStr;       // Pointer to heap-allocated string of characters terminated with 0. Should be freed.

// Widechar UTF16 strings
typedef wChar* Utf16Str;            // Pointer to UTF-16 string terminated with 0.
typedef const wChar* Utf16ConstStr; // Pointer to immutable string of 16 bit characters terminated with 0.
typedef wChar* Utf16HeapStr;        // Pointer to heap-allocated string of 16 bit characters terminated with 0.
typedef wChar* Utf16OwnedStr;       // Pointer to heap-allocated UTF-16 string terminated with 0. Should be freed.

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
