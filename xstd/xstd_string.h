#pragma once

#include "xstd_core.h"
#include "xstd_error.h"
#include "xstd_alloc.h"
#include "xstd_math.h"
#include "xstd_result.h"
#include "xstd_list.h"
#include "xstd_utf8.h"

typedef struct _string_builder
{
    List _strings;
    ibool _valid;
} StringBuilder;

typedef struct _result_strbuilder
{
    StringBuilder value;
    Error error;
} ResultStrBuilder;

/**
 * @brief Calculates the size of a string by traversing it until it meets the
 * string end sentinel.
 * The returned number does not take into consideration the end sentinel.
 * Returns 0 if `s` is NULL
 *
 * ```c
 * ConstStr myString = "Example string";
 * u64 stringSize = string_size(myString); // stringSize == 14
 * ```
 * @param s Terminated string
 * @return u64
 */
static inline u64 string_size(ConstStr s)
{
    if (!s)
        return 0;
    u64 i = 0;
    while (s[i])
    {
        ++i;
    }
    return i;
}

/**
 * @brief Compares two strings and returns !0 (true) if both strings are similar,
 * or 0 (false) if they aren't.
 *
 * ```c
 * ConstStr a = "Example string";
 * ConstStr b = "Example string2"
 * ConstStr c = "Example string";
 * string_equals(a, b); // 0 (false)
 * string_equals(a, c); // !0 (true)
 * ```
 * @param a Terminated string
 * @param b Terminated string
 * @return ibool
 */
static inline ibool string_equals(ConstStr a, ConstStr b)
{
    if (a == b)
        return 1;

    if (!a || !b)
        return 0;

    u64 i = 0;
    while (a[i] == b[i] && a[i])
    {
        ++i;
    }
    return !(a[i] - b[i]);
}

/**
 * @brief Allocates a string of size `sizeNonTerminated` + 1 (for end sentinel)
 * on the heap. The contents of the string is filled with `fill`
 *
 * Memory is owned by the caller and should be freed using the same allocator.
 *
 * ```c
 * ResultOwnedStr myString = string_alloc(default_allocator(), 3, 'w');
 * if (myString.error.code) // Error!
 * // myString.value == ['w', 'w', 'w', 0(end sentinel)]
 * ```
 * @param alloc
 * @param sizeNonTerminated
 * @param fill
 * @return ResultOwnedStr
 */
static inline ResultOwnedStr string_alloc(Allocator *a, u64 sizeNonTerminated, i8 fill)
{
    HeapStr str = (HeapStr)a->alloc(a, sizeNonTerminated + 1);

    if (!str)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "string_alloc", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    u64 i = sizeNonTerminated + 1;

    while (i--)
    {
        str[i] = fill;
    }
    str[sizeNonTerminated] = (i8)0;

    return (ResultOwnedStr){
        .value = str,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Copies a string from `source` to `destination`. Checks if the `destination`
 * is big enough to receive `source`, if not returns `ERR_WOULD_OVERFLOW`.
 *
 * NULL terminates the `destination` string after copying from `source`, meaning the
 * destination string may get smaller.
 * If you don't want this, use `string_copy_n(src, dst, string_size(src))`
 *
 * ```c
 * ConstStr source =     "Example string";
 * String dest = ConstToHeapStr(&alloc, "___________________");
 * Error err = string_copy(source, dest);
 * if (err.code) // Error!
 * // dest == "Example string"
 * ```
 * @param source Terminated string
 * @param destination String of size >= source size
 * @return Error
 */
static inline Error string_copy(ConstStr source, String destination)
{
    if (!source || !destination)
        return X_ERR_EXT("string", "string_copy", ERR_INVALID_PARAMETER, "null arg");

    u64 srcLen = string_size(source);
    u64 destLen = string_size(destination);

    if (destLen < srcLen)
        return X_ERR_EXT("string", "string_copy", ERR_WOULD_OVERFLOW, "dest smaller than src");

    u64 i = 0;
    while (source[i])
    {
        destination[i] = source[i];
        ++i;
    }
    destination[i] = (i8)0;

    return X_ERR_OK;
}

/**
 * @brief Copies `n` characters from `source` to `destination`. Checks if the `destination`
 * is big enough to receive n characters, if not returns `ERR_WOULD_OVERFLOW`.
 *
 * ```c
 * ConstStr source = "Example string";
 * String dest = ConstToHeapStr(&alloc, "_____________");
 * Error err = string_copy_n(source, dest, 5, false);
 * if (err.code) // Error!
 * // dest == "Examp_________"
 * ```
 * @warning DOES NOT TERMINATE `destination` STRING
 * @param source Terminated string
 * @param destination String of size >= n
 * @param terminate if the destination buffer should be NULL terminated after the copied characters.
 * @return Error
 */
static inline Error string_copy_n(ConstStr source, String destination, u64 n, ibool terminate)
{
    if (!source || !destination)
        return X_ERR_EXT("string", "string_copy_n", ERR_INVALID_PARAMETER, "null arg");

    u64 srcLen = string_size(source);
    u64 destLen = string_size(destination);

    if (srcLen < n)
        return X_ERR_EXT("string", "string_copy_n", ERR_WOULD_OVERFLOW, "src smaller than n");

    if (destLen < n)
        return X_ERR_EXT("string", "string_copy_n", ERR_WOULD_OVERFLOW, "dest smaller than n");

    u64 i = 0;
    while (i < n)
    {
        destination[i] = source[i];
        ++i;
    }

    if (terminate)
        destination[i] = (i8)0;

    return X_ERR_OK;
}

/**
 * @brief Unsafe version of `string_copy`.
 * Copies a string from `source` to `destination`. Does not check for overflow,
 * and will cause undefined behavior if `destination` is smaller than `source`.
 *
 * NULL terminates the `destination` string after copying from `source`, meaning the
 * destination string may get smaller.
 * If you don't want this, use `string_copy_n_unsafe(src, dst, string_size(src), false)`
 *
 * ```c
 * ConstStr source =     "Example string";
 * String dest = ConstToHeapStr(&alloc, "_________________");
 * string_copy_unsafe(source, dest);
 * // dest == "Example string"
 * ```
 * @param source Terminated string of size >= n
 * @param destination Terminated string of size >= n
 */
static inline void string_copy_unsafe(ConstStr source, String destination)
{
    u64 i = 0;
    while (source[i])
    {
        destination[i] = source[i];
        ++i;
    }
    destination[i] = (i8)0;
}

/**
 * @brief Unsafe version of `string_copy_n`.
 * Copies `n` characters from `source` to `destination`. Does not check for overflow,
 * nor does it validate arguments.
 * Will cause buffer overflow if `destination` is smaller than `n` or if
 * `source` is smaller than `n`.
 *
 * ```c
 * ConstStr source = "Example string";
 * String dest = ConstToHeapStr(&alloc, "_____________");
 * string_copy_n_unsafe(source, dest, 5, false);
 * // dest == "Examp_________"
 * ```
 * @param source Terminated string of size >= n
 * @param destination Terminated string of size >= n
 * @param terminate if the destination buffer should be NULL terminated after the copied characters.
 */
static inline void string_copy_n_unsafe(ConstStr source, String destination, u64 n, ibool terminate)
{
    u64 i = 0;
    while (i < n)
    {
        destination[i] = source[i];
        ++i;
    }
    if (terminate)
        destination[i] = (i8)0;
}

/**
 * @brief Allocates and creates a copy of `source` on the heap.
 *
 * Memory is owned by the caller and should be freed using the same allocator.
 *
 * ```c
 * ConstStr source = "Example string";
 * ResultOwnedStr newString = string_dupe(default_allocator(), source);
 * if (newString.error.code) // Error!
 * // newString.value == "Example string"
 * ```
 * @param alloc
 * @param source Terminated string
 * @return String
 */
static inline ResultOwnedStr string_dupe(Allocator *a, ConstStr source)
{
    if (!source)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "string_dupe", ERR_WOULD_OVERFLOW, "null src"),
        };

    u64 srcLen = string_size(source);
    HeapStr newStr = (HeapStr)a->alloc(a, srcLen + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "string_dupe", ERR_WOULD_OVERFLOW, "alloc failure"),
        };

    string_copy_unsafe(source, newStr);

    return (ResultOwnedStr){
        .value = newStr,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Extracts a substring from a UTF-8 or ASCII string using codepoint indices.
 * Memory is owned by the caller and must be released with the provided allocator.
 * @warning `start` and `end` are counted in codepoints rather than raw bytes.
 *
 * ```c
 * ConstStr src = "Cafe\xCC\x81 Gott";
 * ResultOwnedStr res = string_substr(default_allocator(), src, 0, 5);
 * if (res.error.code) // Handle error
 * // res.value == "Cafe\xCC\x81"
 * ```
 * @param a Allocator that will own the resulting string
 * @param s UTF-8 or ASCII encoded source string
 * @param start Inclusive codepoint index
 * @param end Exclusive codepoint index
 * @return ResultOwnedStr
 */
static inline ResultOwnedStr string_substr(Allocator *a, ConstStr s, u64 start, u64 end)
{
    if (!a || !s)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("string", "string_substr", ERR_INVALID_PARAMETER, "null arg"),
        };

    if (end < start)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("string", "string_substr", ERR_INVALID_PARAMETER, "end smaller than start"),
        };

    ResultUtf8Iter iterRes = utf8_iter_str(s);
    if (iterRes.error.code != ERR_OK)
        return (ResultOwnedStr){
            .error = iterRes.error,
        };

    Utf8Iter startIt = iterRes.value;

    for (u64 i = 0; i < start; ++i)
    {
        ResultUtf8Codepoint cpRes = utf8_iter_next(&startIt);
        if (cpRes.error.code != ERR_OK)
            return (ResultOwnedStr){
                .error = X_ERR_EXT("string", "string_substr", ERR_INVALID_PARAMETER, "start out of string bounds"),
            };
    }

    Utf8Iter endIt = startIt;
    for (u64 i = start; i < end; ++i)
    {
        ResultUtf8Codepoint cpRes = utf8_iter_next(&endIt);
        if (cpRes.error.code != ERR_OK)
            return (ResultOwnedStr){
                .error = X_ERR_EXT("string", "string_substr", ERR_INVALID_PARAMETER, "end out of string bounds"),
            };
    }

    u64 byteCount = (u64)(endIt.ptr - startIt.ptr);
    HeapStr newStr = (HeapStr)a->alloc(a, byteCount + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("string", "string_substr", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    string_copy_n_unsafe(startIt.ptr, newStr, byteCount, false);
    newStr[byteCount] = 0;

    return (ResultOwnedStr){
        .value = newStr,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Allocates and creates a copy of `source` on the heap.
 *
 * Memory is owned by the caller and should be freed using the same allocator.
 * May return NULL on allocation failure.
 *
 * ```c
 * ConstStr source = "Example string";
 * OwnedStr newString = string_dupe_noresult(default_allocator(), source);
 * if (!newString) // Error!
 * // newString == "Example string"
 * ```
 * @param alloc
 * @param source Terminated string
 * @return String
 */
static inline OwnedStr string_dupe_noresult(Allocator *a, ConstStr source)
{
    if (!source)
        return NULL;

    u64 srcLen = string_size(source);
    HeapStr newStr = (HeapStr)a->alloc(a, srcLen + 1);

    if (!newStr)
        return NULL;

    string_copy_unsafe(source, newStr);
    return newStr;
}

// Converts a const char* to OwnedStr, uses default allocator.
// TODO: take allocator instead of defaulting to c_alloc
#define ConstToHeapStr(allocPtr, constStr) string_dupe_noresult((allocPtr), (constStr))

/**
 * @brief Creates a copy of the string with a new size. Copies as much of the contents
 * of the original string back to the new allocated string and fills the rest with `fill`.
 * Always terminates the new string.
 *
 * Memory is owned by the caller and should be freed using the same allocator.
 *
 * ```c
 * ConstStr myString = "str";
 * ResultOwnedStr newStr = string_resize(default_allocator(), myString, 7, 'w');
 * if (newStr.error.code) // Error!
 * // newStr.value == ['s','t','r','w','w','w','w',0(end sentinel)]
 * ResultOwnedStr newStr2 = string_resize(default_allocator(), myString, 2, 'w');
 * if (newStr2.error.code) // Error!
 * // newStr2.value == ['s','t',0(end sentinel)]
 * ```
 * @param alloc
 * @param s Terminated string
 * @param newSizeNonTerminated Size of the new string
 * @param fill character to fill the empty allocated space
 */
static inline ResultOwnedStr string_resize(Allocator *a, ConstStr source, u64 newSizeNonTerminated, i8 fill)
{
    if (!source)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "string_resize", ERR_INVALID_PARAMETER, "null source"),
        };

    u64 prevSize = string_size(source);
    u64 cpySize = prevSize < newSizeNonTerminated ? prevSize : newSizeNonTerminated;
    u64 fillCnt = newSizeNonTerminated - cpySize;
    HeapStr newStr = (HeapStr)a->alloc(a, newSizeNonTerminated + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "string_resize", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    string_copy_n_unsafe(source, newStr, cpySize, false);

    for (u64 i = 0; i < fillCnt; ++i)
    {
        newStr[cpySize + i] = fill;
    }
    newStr[newSizeNonTerminated] = (i8)0;

    return (ResultOwnedStr){
        .value = newStr,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Concatenates both strings into a single newly allocated string.
 * If you need to concatenate more than 2 strings, consider using a StringBuilder.
 *
 * Memory is owned by the caller and should be freed using the same allocator.
 *
 * ```c
 * ConstStr a = "Hello, ";
 * ConstStr b = "World!"
 * ResultOwnedStr newStr = string_concat(default_allocator(), a, b);
 * if (newStr.error.code) // Error!
 * // newStr.value == "Hello, World!"
 * ```
 * @param alloc
 * @param a
 * @param b
 * @return String
 */
static inline ResultOwnedStr string_concat(Allocator *al, ConstStr a, ConstStr b)
{
    if (!al || !a || !b)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "string_concat", ERR_INVALID_PARAMETER, "null arg"),
        };

    u64 aLen = string_size(a);
    u64 bLen = string_size(b);
    HeapStr newStr = (HeapStr)al->alloc(al, aLen + bLen + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "string_concat", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    string_copy_unsafe(a, newStr);
    string_copy_unsafe(b, newStr + aLen);

    return (ResultOwnedStr){
        .value = newStr,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Retrieves the UTF-8 codepoint at the given codepoint index. Works for both UTF-8 and ASCII strings.
 * @warning call to string_at triggers a full traversal of the string up to
 * the codepoint index, if you are using it in a loop, prefer using `Utf8Iter`
 * and `utf8_iter_next()` from `xstd_utf8.h`
 *
 * ```c
 * ConstStr text = "na\xC3\xAFve";
 * ResultUtf8Codepoint cp = string_at(text, 2);
 * if (cp.error.code) // Handle error
 * // cp.value.codepoint == 0xEF
 * ```
 * @param s UTF-8 encoded string
 * @param index Codepoint index to fetch
 * @return ResultUtf8Codepoint
 */
static inline ResultUtf8Codepoint string_char_at(ConstStr s, u64 index)
{
    if (!s)
        return (ResultUtf8Codepoint){
            .error = X_ERR_EXT("string", "string_at", ERR_INVALID_PARAMETER, "null string"),
        };

    ResultUtf8Iter iterRes = utf8_iter_str(s);
    if (iterRes.error.code != ERR_OK)
        return (ResultUtf8Codepoint){
            .value = {0},
            .error = iterRes.error,
        };

    Utf8Iter it = iterRes.value;

    for (u64 i = 0; i < index; ++i)
    {
        ResultUtf8Codepoint step = utf8_iter_next(&it);
        if (step.error.code != ERR_OK)
            return (ResultUtf8Codepoint){
                .error = X_ERR_EXT("string", "string_at", ERR_RANGE_ERROR, "index out of bounds"),
            };
    }

    ResultUtf8Codepoint cpRes = utf8_iter_peek(&it);
    if (cpRes.error.code != ERR_OK)
        return (ResultUtf8Codepoint){
            .error = X_ERR_EXT("string", "string_at", ERR_RANGE_ERROR, "index out of bounds"),
        };

    return cpRes;
}

/**
 * @brief Retrieves the ASCII character at the given byte index.
 *
 * ```c
 * ConstStr text = "example";
 * ResultByte ch = string_at_ascii(text, 3, 7);
 * if (ch.error.code) // Handle error
 * // ch.value == 'm'
 * ```
 * @param s ASCII string
 * @param index Zero-based byte index
 * @param stringSizeBytes Caller-provided string length in bytes
 * @return ResultByte
 */
static inline ResultByte string_char_at_ascii(ConstStr s, u64 index, u64 stringSizeBytes)
{
    if (!s)
        return (ResultByte){
            .value = 0,
            .error = X_ERR_EXT("string", "string_at_ascii", ERR_INVALID_PARAMETER, "null string"),
        };

    if (index >= stringSizeBytes)
        return (ResultByte){
            .value = 0,
            .error = X_ERR_EXT("string", "string_at_ascii", ERR_RANGE_ERROR, "index out of bounds"),
        };

    return (ResultByte){
        .value = s[index],
        .error = X_ERR_OK,
    };
}

/**
 * @brief Extracts a substring from another string.
 * `start` is inclusive and `end` is exclusive.
 *
 * Memory is owned by the caller and should be freed using the same allocator.
 *
 * ```c
 * ConstStr myStr = "Hello, World!";
 * ResultOwnedStr subStr = string_substr_ascii(default_allocator(), myStr, 7, 12);
 * if (subStr.error.code) // Error!
 * // subStr.value == "World"
 * ```
 * @param alloc
 * @param s
 * @param start
 * @param end
 * @return ResultOwnedStr
 */
static inline ResultOwnedStr string_substr_ascii(Allocator *a, ConstStr s, u64 start, u64 end)
{
    if (!a || !s)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("string", "string_substr_ascii", ERR_INVALID_PARAMETER, "null arg"),
        };

    if (end < start)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("string", "string_substr_ascii", ERR_INVALID_PARAMETER, "end smaller than start"),
        };

    u64 strSize = string_size(s);
    if (start > strSize || end > strSize)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("string", "string_substr_ascii", ERR_INVALID_PARAMETER, "start/end out of string bounds"),
        };

    u64 subSize = end - start;
    HeapStr newStr = (HeapStr)a->alloc(a, subSize + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("string", "string_substr_ascii", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    string_copy_n_unsafe(s + start, newStr, subSize, false);
    newStr[subSize] = (i8)0;

    return (ResultOwnedStr){
        .value = newStr,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Extracts a substring from another string.
 * Both `start` is inclusive and `end` is exclusive.
 *
 * Memory is owned by the caller and should be freed using the same allocator.
 * Does not check bounds or any validations.
 *
 * ```c
 * ConstStr myStr = "Hello, World!";
 * OwnedStr subStr = string_substr_ascii_unsafe(default_allocator(), myStr, 7, 12);
 * if (!subStr) // Error!
 * // subStr == "World"
 * ```
 * @param alloc
 * @param s
 * @param start
 * @param end
 * @return OwnedStr
 */
static inline OwnedStr string_substr_ascii_unsafe(Allocator *a, ConstStr s, u64 start, u64 end)
{
    u64 subSize = end - start;
    HeapStr newStr = (HeapStr)a->alloc(a, subSize + 1);

    if (!newStr)
        return NULL;

    string_copy_n_unsafe(s + start, newStr, subSize, false);
    newStr[subSize] = (i8)0;
    return newStr;
}

/**
 * @brief Extracts a substring from a UTF-8 or ASCII string without bounds checks.
 * Memory is owned by the caller and must be released with the same allocator.
 * @warning Caller must guarantee that `start` and `end` are valid codepoint offsets.
 *
 * ```c
 * ConstStr src = "Buenos d\xC3\xADas";
 * OwnedStr sub = string_substr_unsafe(default_allocator(), src, 7, 11);
 * if (!sub) // Handle allocation failure
 * // sub == "d\xC3\xADas"
 * ```
 * @param a Allocator that will own the resulting string
 * @param s UTF-8 encoded source string
 * @param start Inclusive codepoint index
 * @param end Exclusive codepoint index
 * @return OwnedStr
 */
static inline OwnedStr string_substr_unsafe(Allocator *a, ConstStr s, u64 start, u64 end)
{
    if (!a || !s)
        return NULL;

    ResultUtf8Iter iterRes = utf8_iter_str(s);
    if (iterRes.error.code != ERR_OK)
        return NULL;

    Utf8Iter it = iterRes.value;

    for (u64 i = 0; i < start; ++i)
        (void)utf8_iter_next(&it);

    ConstStr startPtr = it.ptr;

    for (u64 i = start; i < end; ++i)
        (void)utf8_iter_next(&it);

    u64 byteCount = (u64)(it.ptr - startPtr);
    HeapStr newStr = (HeapStr)a->alloc(a, byteCount + 1);
    if (!newStr)
        return NULL;

    string_copy_n_unsafe(startPtr, newStr, byteCount, false);
    newStr[byteCount] = 0;
    return newStr;
}

/**
 * @brief Splits a string on every occurrence of `delimiter`.
 *
 * Memory of both strings and the list is owned by the caller and should be freed using the same allocator.
 *
 * ```c
 * ConstStr myStr = "Hello, World!";
 * ResultList split = string_split_char_ascii(default_allocator(), myStr, ' ');
 * if (split.error.code) // Error!
 * // split.value == List["Hello,", "World!"]
 * List l = res.value;
 * // Do list stuff
 * list_free_items(default_allocator(), &l); // Frees all allocated strings
 * list_deinit(&l); // Free the list
 * ```
 * @param alloc
 * @param s
 * @param delimiter
 * @return String
 */
static inline ResultList string_split_char_ascii(Allocator *alloc, ConstStr s, i8 delimiter)
{
    // What a mess...

    if (!alloc || !s)
        return (ResultList){
            .error = X_ERR_EXT("string", "string_split_char_ascii", ERR_INVALID_PARAMETER, "null arg"),
        };

    ResultList resList = ListInitT(HeapStr, alloc);

    if (resList.error.code != ERR_OK)
        return resList;

    List l = resList.value;

    u64 bound = string_size(s);
    u64 i = 0;
    u64 segStart = i;

    while (s[i])
    {
        if (s[i] == delimiter)
        {
            HeapStr subStr = string_substr_ascii_unsafe(alloc, s, segStart, i);

            if (!subStr)
                return (ResultList){
                    .error = X_ERR_EXT("string", "string_split_char_ascii", ERR_OUT_OF_MEMORY, "alloc failure"),
                };

            list_push(&l, &subStr);

            ++i;
            segStart = i;
            continue;
        }

        ++i;
    }

    HeapStr subStr = string_substr_ascii_unsafe(alloc, s, segStart, bound);
    if (!subStr)
        return (ResultList){
            .error = X_ERR_EXT("string", "string_split_char_ascii", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    list_push(&l, &subStr);

    return (ResultList){
        .value = l,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Splits a UTF-8 string by a single-byte delimiter.
 * Memory for both the list and the produced strings belongs to the caller.
 *
 * ```c
 * ConstStr text = "emoji|\xF0\x9F\x98\x80";
 * ResultList res = string_split_char(default_allocator(), text, '|');
 * if (res.error.code) // Handle error
 * // res.value == List["emoji", "\xF0\x9F\x98\x80"]
 * ```
 * @param alloc Allocator used for the resulting list and substrings
 * @param s UTF-8 encoded source string
 * @param delimiter ASCII delimiter byte
 * @return ResultList
 */
static inline ResultList string_split_char(Allocator *alloc, ConstStr s, i8 delimiter)
{
    if (!alloc || !s)
        return (ResultList){
            .value = {0},
            .error = X_ERR_EXT("string", "string_split_char", ERR_INVALID_PARAMETER, "null arg"),
        };

    ResultList resList = ListInitT(HeapStr, alloc);

    if (resList.error.code != ERR_OK)
        return resList;

    List l = resList.value;

    ResultUtf8Iter iterRes = utf8_iter_str(s);
    if (iterRes.error.code != ERR_OK)
        return (ResultList){
            .value = {0},
            .error = iterRes.error,
        };

    Utf8Iter it = iterRes.value;
    ConstStr segmentStart = it.ptr;

    while (utf8_iter_has_next(&it))
    {
        ConstStr current = it.ptr;
        ResultUtf8Codepoint cpRes = utf8_iter_next(&it);

        if (cpRes.error.code != ERR_OK)
            return (ResultList){
                .value = {0},
                .error = cpRes.error,
            };

        if (cpRes.value.codepoint == (u32)(u8)delimiter)
        {
            u64 segLen = (u64)(current - segmentStart);
            HeapStr subStr = (HeapStr)alloc->alloc(alloc, segLen + 1);

            if (!subStr)
                return (ResultList){
                    .value = {0},
                    .error = X_ERR_EXT("string", "string_split_char", ERR_OUT_OF_MEMORY, "alloc failure"),
                };

            string_copy_n_unsafe(segmentStart, subStr, segLen, false);
            subStr[segLen] = 0;
            list_push(&l, &subStr);
            segmentStart = it.ptr;
        }
    }

    u64 segLen = (u64)(it.ptr - segmentStart);
    HeapStr subStr = (HeapStr)alloc->alloc(alloc, segLen + 1);

    if (!subStr)
        return (ResultList){
            .value = {0},
            .error = X_ERR_EXT("string", "string_split_char", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    string_copy_n_unsafe(segmentStart, subStr, segLen, false);
    subStr[segLen] = 0;
    list_push(&l, &subStr);

    return (ResultList){
        .value = l,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Splits a string on every occurrence of `\r\n` or `\n`.
 *
 * Memory of both strings and the list is owned by the caller and should be freed using the same allocator.
 *
 * ```c
 * ConstStr myStr = "Hello,\nWorld!";
 * ResultList split = string_split_lines_ascii(default_allocator(), myStr);
 * if (split.error.code) // Error!
 * // split.value == List["Hello,", "World!"]
 * List l = res.value;
 * // Do list stuff
 * list_free_items(default_allocator(), &l); // Frees all allocated strings
 * list_deinit(&l); // Free the list
 * ```
 * @param alloc
 * @param s
 * @return ResultList
 */
static inline ResultList string_split_lines_ascii(Allocator *alloc, ConstStr s)
{
    // What a mess...

    if (!alloc || !s)
        return (ResultList){
            .value = {0},
            .error = X_ERR_EXT("string", "string_split_lines_ascii", ERR_INVALID_PARAMETER, "null arg"),
        };

    ResultList resList = ListInitT(HeapStr, alloc);

    if (resList.error.code != ERR_OK)
        return resList;

    List l = resList.value;

    u64 bound = string_size(s);
    u64 i = 0;
    u64 segStart = i;

    while (s[i])
    {
        if (s[i] == '\n' || (s[i] == '\r' && s[i + 1] == '\n'))
        {
            HeapStr subStr = string_substr_ascii_unsafe(alloc, s, segStart, i);

            if (!subStr)
                return (ResultList){
                    .value = {0},
                    .error = X_ERR_EXT("string", "string_split_lines_ascii", ERR_OUT_OF_MEMORY, "alloc failure"),
                };

            list_push(&l, &subStr);

            if (s[i] == '\r')
                ++i;

            ++i;
            segStart = i;
            continue;
        }

        ++i;
    }

    HeapStr subStr = string_substr_ascii_unsafe(alloc, s, segStart, bound);

    if (!subStr)
        return (ResultList){
            .value = {0},
            .error = X_ERR_EXT("string", "string_split_lines_ascii", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    list_push(&l, &subStr);

    return (ResultList){
        .value = l,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Splits a UTF-8 or ASCII string on newline sequences.
 * Memory for both the list and the produced strings belongs to the caller.
 *
 * ```c
 * ConstStr text = "line1\nline2\r\nline3";
 * ResultList res = string_split_lines(default_allocator(), text);
 * if (res.error.code) // Handle error
 * // res.value == List["line1", "line2", "line3"]
 * ```
 * @param alloc Allocator used for the resulting list and substrings
 * @param s UTF-8 or ASCII encoded source string
 * @return ResultList
 */
static inline ResultList string_split_lines(Allocator *alloc, ConstStr s)
{
    if (!alloc || !s)
        return (ResultList){
            .value = {0},
            .error = X_ERR_EXT("string", "string_split_lines", ERR_INVALID_PARAMETER, "null arg"),
        };

    ResultList resList = ListInitT(HeapStr, alloc);
    if (resList.error.code != ERR_OK)
        return resList;

    List l = resList.value;

    ResultUtf8Iter iterRes = utf8_iter_str(s);
    if (iterRes.error.code != ERR_OK)
        return (ResultList){
            .value = {0},
            .error = iterRes.error,
        };

    Utf8Iter it = iterRes.value;
    ConstStr segmentStart = it.ptr;

    while (utf8_iter_has_next(&it))
    {
        ConstStr current = it.ptr;
        ResultUtf8Codepoint cpRes = utf8_iter_next(&it);

        if (cpRes.error.code != ERR_OK)
            return (ResultList){
                .value = {0},
                .error = cpRes.error,
            };

        if (cpRes.value.codepoint == '\n' || cpRes.value.codepoint == '\r')
        {
            if (cpRes.value.codepoint == '\r')
            {
                ResultUtf8Codepoint peekRes = utf8_iter_peek(&it);
                if (peekRes.error.code == ERR_OK && peekRes.value.codepoint == '\n')
                    utf8_iter_next(&it);
            }

            u64 segLen = (u64)(current - segmentStart);
            HeapStr subStr = (HeapStr)alloc->alloc(alloc, segLen + 1);

            if (!subStr)
                return (ResultList){
                    .value = {0},
                    .error = X_ERR_EXT("string", "string_split_lines", ERR_OUT_OF_MEMORY, "alloc failure"),
                };

            string_copy_n_unsafe(segmentStart, subStr, segLen, false);
            subStr[segLen] = 0;
            list_push(&l, &subStr);
            segmentStart = it.ptr;
        }
    }

    u64 segLen = (u64)(it.ptr - segmentStart);
    HeapStr subStr = (HeapStr)alloc->alloc(alloc, segLen + 1);

    if (!subStr)
        return (ResultList){
            .value = {0},
            .error = X_ERR_EXT("string", "string_split_lines", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    string_copy_n_unsafe(segmentStart, subStr, segLen, false);
    subStr[segLen] = 0;
    list_push(&l, &subStr);

    return (ResultList){
        .value = l,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Returns the index of the first occurrence of `needle` within `haystack`
 * If no occurrences have been found, returns -1;
 *
 * @param haystack
 * @param needle
 * @return i64
 */
static inline i64 string_find_char_ascii(ConstStr haystack, i8 needle)
{
    if (!haystack || !needle)
        return -1;

    u64 i = 0;
    while (haystack[i])
    {
        if (haystack[i] == needle)
            return (i64)i;

        ++i;
    }
    return -1;
}

/**
 * @brief Finds the first occurrence of an ASCII byte in a UTF-8 string.
 *
 * ```c
 * ConstStr text = "Cafe\xCC\x81 au lait";
 * i64 idx = string_find_char(text, ' ');
 * // idx == 5
 * ```
 * @param haystack UTF-8 encoded string to search
 * @param needle ASCII byte to find
 * @return i64
 */
static inline i64 string_find_char(ConstStr haystack, i8 needle)
{
    if (!haystack || !needle)
        return -1;

    ResultUtf8Iter iterRes = utf8_iter_str(haystack);
    if (iterRes.error.code != ERR_OK)
        return -1;

    Utf8Iter it = iterRes.value;
    ConstStr base = it.ptr;

    while (utf8_iter_has_next(&it))
    {
        ConstStr current = it.ptr;
        ResultUtf8Codepoint cpRes = utf8_iter_next(&it);
        if (cpRes.error.code != ERR_OK)
            return -1;

        if (cpRes.value.codepoint == (u32)(u8)needle)
            return (i64)(current - base);
    }

    return -1;
}

/**
 * @brief Returns the index of the first occurrence of `needle` within `haystack`
 * If no occurrences have been found, returns -1;
 *
 * @param haystack
 * @param needle
 * @return i64
 */
static inline i64 string_find(ConstStr haystack, ConstStr needle)
{
    if (!haystack || !needle)
        return -1;

    u64 i = 0;
    while (haystack[i])
    {
        u64 j = 0;
        while (haystack[i + j] && needle[j] && haystack[i + j] == needle[j])
        {
            ++j;
        }

        if (!needle[j])
            return (i64)i;

        ++i;
    }
    return -1;
}

/**
 * @brief Creates a StringBuilder, a useful interface for string concatenation.
 *
 * `strbuilder_deinit()` should be called after use.
 *
 * ```c
 * ResultStrBuilder resBuilder = strbuilder_init(&alloc);
 * if (resBuilder.error.code) // Error!
 * StringBuilder builder = resBuilder.value;
 * strbuilder_push_copy(&builder, "Hello,");
 * strbuilder_push_owned(&builder, ConstToHeapStr(&alloc, " World!"));
 * ResultOwnedStr resBuiltStr = strbuilder_get_string(&builder);
 * if (resBuiltStr.error.code) // Error!
 * OwnedStr builtStr = resBuiltStr.value;
 * strbuilder_deinit(&builder);
 * ```
 * @param alloc
 * @return ResultStrBuilder
 */
static inline ResultStrBuilder strbuilder_init(Allocator *alloc)
{
    ResultList res = list_init(alloc, sizeof(HeapStr), 16);

    if (res.error.code)
        return (ResultStrBuilder){
            .value = {
                ._valid = false,
            },
            .error = res.error,
        };

    return (ResultStrBuilder){
        .value = {
            ._strings = res.value,
            ._valid = true,
        },
        .error = X_ERR_OK,
    };
}

/**
 * @brief Clears the underlying data of the StringBuilder. Builder can be reused
 * after call to this function.
 *
 * @param builder
 */
static inline void strbuilder_clear(StringBuilder *builder)
{
    if (!builder || !builder->_valid)
        return;

    list_free_items(&builder->_strings._allocator, &builder->_strings);
    list_clear_nofree(&builder->_strings);
}

/**
 * @brief Releases the memory allocated by the StringBuilder.
 * The builder cannot be reused after call to this function.
 *
 * @param builder
 */
static inline void strbuilder_deinit(StringBuilder *builder)
{
    if (!builder || !builder->_valid)
        return;

    builder->_valid = false;

    list_free_items(&builder->_strings._allocator, &builder->_strings);
    list_deinit(&builder->_strings);
}

/**
 * @brief Pushes `s` to the list of strings inside the StringBuilder.
 *
 * Caller loses the ownership of the string, once `strbuilder_deinit` is called,
 * `s` will be freed.
 *
 * `s` must have been allocated using the same allocator use for call to `strbuilder_init`.
 * If you call this function with a constant string, it WILL crash the program.
 *
 * `s` should not be used after a call to this function as doing so would be undefined behavior.
 *
 * @param builder
 * @param s
 */
static inline void strbuilder_push_owned(StringBuilder *builder, OwnedStr s)
{
    if (!builder || !builder->_valid)
        return;

    if (!s)
        return;

    list_push(&builder->_strings, &s);
}

/**
 * @brief Pushes a copy `s` to the list of strings inside the StringBuilder.
 *
 * @param builder
 * @param s
 */
static inline void strbuilder_push_copy(StringBuilder *builder, ConstStr s)
{
    if (!builder || !builder->_valid)
        return;

    if (!s)
        return;

    HeapStr copy = string_dupe_noresult(&builder->_strings._allocator, s);
    if (!copy)
        return;

    list_push(&builder->_strings, &copy);
}

/**
 * @brief Appends all strings inside builder as a newly allocated string.
 *
 * Memory is owned by the caller and should be freed after use.
 *
 * ```c
 * ResultStrBuilder res = strbuilder_init(default_allocator());
 * if (res.error.code) // Error!
 * StringBuilder builder = res.value;
 * strbuilder_push_copy(&builder, "Hello,");
 * strbuilder_push_owned(&builder, ConstToHeapStr(default_allocator(), "World!"));
 * ResultOwnedStr built = strbuilder_get_string(&builder);
 * if (built.error.code) // Error!
 * OwnedStr resultStr = built.value;
 * strbuilder_deinit(&builder);
 * ```
 * @param builder
 * @return ResultOwnedStr
 */
static inline ResultOwnedStr strbuilder_get_string(StringBuilder *builder)
{
    if (!builder || !builder->_valid)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "strbuilder_get_string", ERR_INVALID_PARAMETER, "null or invalid builder"),
        };

    u64 totalSize = 0;
    u64 bound = list_size(&builder->_strings);

    for (u64 i = 0; i < bound; ++i)
    {
        HeapStr s = (HeapStr)list_get_as_ptr(&builder->_strings, i);
        totalSize += string_size(s);
    }

    Allocator *a = &builder->_strings._allocator;
    HeapStr newStr = (HeapStr)a->alloc(a, totalSize + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "strbuilder_get_string", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    u64 offset = 0;
    for (u64 i = 0; i < bound; ++i)
    {
        HeapStr s = (HeapStr)list_get_as_ptr(&builder->_strings, i);
        HeapStr source = s;

        u64 j = 0;
        while (source[j])
        {
            newStr[offset] = source[j];
            ++j;
            ++offset;
        }
    }

    newStr[totalSize] = (i8)0;
    return (ResultOwnedStr){
        .value = newStr,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Replaces every occurrences of `what` within `s` with `with`.
 *
 * Memory is owned by the caller and should be freed after use.
 *
 * ```c
 * ResultOwnedStr res = string_replace(&alloc, "This is a test, or is it?", "is", "is not");
 * if (res.error.code) // Error!
 * OwnedStr replaced = res.value;
 * // replaced == "This is not a test, or is not it?"
 * ```
 * @param alloc
 * @param s
 * @param what
 * @param with
 * @return ResultOwnedStr
 */
static inline ResultOwnedStr string_replace(Allocator *alloc, ConstStr s, ConstStr what, ConstStr with)
{
    if (!alloc || !s || !what || !with)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "string_replace", ERR_INVALID_PARAMETER, "null arg"),
        };

    ResultStrBuilder strBldRes = strbuilder_init(alloc);

    if (strBldRes.error.code != ERR_OK)
        return (ResultOwnedStr){
            .value = NULL,
            .error = strBldRes.error,
        };

    StringBuilder strBld = strBldRes.value;

    u64 whatSize = string_size(what);

    i64 next = -1;
    u64 offset = 0;

    if (whatSize != 0)
    {
        while ((next = string_find(s + offset, what)) != -1)
        {
            HeapStr chunk = string_substr_ascii_unsafe(alloc, s + offset, 0, (u64)next);
            if (!chunk)
                return (ResultOwnedStr){
                    .value = NULL,
                    .error = X_ERR_EXT("string", "string_replace", ERR_OUT_OF_MEMORY, "alloc failure"),
                };

            strbuilder_push_owned(&strBld, chunk);

            HeapStr replaced = string_dupe_noresult(alloc, with);
            strbuilder_push_owned(&strBld, replaced);

            offset += (u64)next + whatSize;
        }
    }

    HeapStr tailChunk = string_dupe_noresult(alloc, s + offset);
    if (!tailChunk)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "string_replace", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    strbuilder_push_owned(&strBld, tailChunk);

    ResultOwnedStr strRes = strbuilder_get_string(&strBld);

    if (strRes.error.code)
        return (ResultOwnedStr){
            .value = NULL,
            .error = strRes.error,
        };

    HeapStr result = strRes.value;

    strbuilder_deinit(&strBld);

    return (ResultOwnedStr){
        .value = result,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Checks if string `s` starts with substring `what`.
 * Returns !0 (true) if `s` starts with `what`, or 0 (false) otherwise.
 *
 * ```c
 * ConstStr str = "Example string";
 * string_starts_with(str, "Exam"); // !0 (true)
 * string_starts_with(str, "exam"); // 0 (false)
 * ```
 * @param s Terminated string to check
 * @param what Terminated string to find at the beginning of `s`
 * @return ibool
 */
static inline ibool string_starts_with(ConstStr s, ConstStr what)
{
    if (!s || !what)
        return 0;

    if (!what[0])
        return 1;

    u64 i = 0;
    while (s[i] && what[i] && s[i] == what[i])
    {
        ++i;
    }
    return what[i] == 0;
}

/**
 * @brief Checks if string `s` ends with substring `what`.
 * Returns !0 (true) if `s` ends with `what`, or 0 (false) otherwise.
 *
 * ```c
 * ConstStr str = "Example string";
 * string_ends_with(str, "string"); // !0 (true)
 * string_ends_with(str, "String"); // 0 (false)
 * ```
 * @param s Terminated string to check
 * @param what Terminated string to find at the end of `s`
 * @return ibool
 */
static inline ibool string_ends_with(ConstStr s, ConstStr what)
{
    if (!s || !what)
        return 0;

    if (!what[0])
        return 1;

    u64 sLen = string_size(s);
    u64 whatLen = string_size(what);

    if (whatLen > sLen)
        return 0;

    for (u64 i = 0; i < whatLen; ++i)
    {
        if (s[sLen - whatLen + i] != what[i])
            return 0;
    }
    return 1;
}

/**
 * @brief Checks if character is alphabetic (in range a-z A-Z)
 *
 * @param c character
 * @return ibool
 */
static inline ibool char_is_alpha_ascii(const i8 c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

/**
 * @brief Checks if character is numerical (in range 0-9)
 *
 * @param c character
 * @return ibool
 */
static inline ibool char_is_digit_ascii(const i8 c)
{
    return (c >= '0' && c <= '9');
}

/**
 * @brief Checks if character is alphanumerical (in range a-z A-Z 0-9)
 *
 * @param c
 * @return ibool
 */
static inline ibool char_is_alphanum_ascii(const i8 c)
{
    return (char_is_alpha_ascii(c) || char_is_digit_ascii(c));
}

/**
 * @brief Checks if character is whitespace (space, tab, newline, carriage return)
 *
 * @param c
 * @return ibool
 */
static inline ibool char_is_whitespace_ascii(const i8 c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

/**
 * @brief Returns a lowercase version of a uppercase character,
 * returns provided character if not lowercase.
 *
 * @param c
 * @return i8
 */
static inline i8 char_to_lower_ascii(const i8 c)
{
    if (c >= 'A' && c <= 'Z')
        return (i8)(c + ('a' - 'A'));
    return c;
}

/**
 * @brief Returns a uppercase version of a lowercase character,
 * returns provided character if not lowercase.
 *
 * @param c
 * @return i8
 */
static inline i8 char_to_upper_ascii(const i8 c)
{
    if (c >= 'a' && c <= 'z')
        return (i8)(c - ('a' - 'A'));
    return c;
}

static inline ibool _utf8_is_ascii_digit(u32 codepoint)
{
    return codepoint >= '0' && codepoint <= '9';
}

static inline ibool _utf8_is_whitespace_cp(u32 codepoint)
{
    switch (codepoint)
    {
    case 0x0009: // TAB
    case 0x000A: // LF
    case 0x000B: // VT
    case 0x000C: // FF
    case 0x000D: // CR
    case 0x001C:
    case 0x001D:
    case 0x001E:
    case 0x001F:
    case 0x0020:
    case 0x0085:
    case 0x00A0:
    case 0x1680:
    case 0x2000:
    case 0x2001:
    case 0x2002:
    case 0x2003:
    case 0x2004:
    case 0x2005:
    case 0x2006:
    case 0x2007:
    case 0x2008:
    case 0x2009:
    case 0x200A:
    case 0x2028:
    case 0x2029:
    case 0x202F:
    case 0x205F:
    case 0x3000:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Performs a in-place replacement of uppercase characters with lowercase ones.
 * Provided string MUST be modifiable and non const.
 *
 * @param s
 */
static inline void string_to_lower_inplace_ascii(HeapStr s)
{
    if (!s)
        return;

    u64 i = 0;
    while (s[i])
    {
        s[i] = char_to_lower_ascii(s[i]);
        ++i;
    }
}

static inline ibool char_is_alpha(const i8 c)
{
    return char_is_alpha_ascii(c);
}

static inline ibool char_is_digit(const i8 c)
{
    return char_is_digit_ascii(c);
}

static inline ibool char_is_alphanum(const i8 c)
{
    return char_is_alphanum_ascii(c);
}

static inline ibool char_is_whitespace(const i8 c)
{
    return char_is_whitespace_ascii(c);
}

static inline i8 char_to_lower(const i8 c)
{
    return char_to_lower_ascii(c);
}

static inline i8 char_to_upper(const i8 c)
{
    return char_to_upper_ascii(c);
}

/**
 * @brief Converts a UTF-8 string to lowercase in place (ASCII only).
 * Modifies the provided buffer directly.
 *
 * ```c
 * HeapStr text = ConstToHeapStr(default_allocator(), "CAF\xC3\x89");
 * string_to_lower_inplace(text);
 * // text == "caf\xC3\xA9"
 * ```
 * @param s Mutable UTF-8 string buffer
 */
static inline void string_to_lower_inplace(HeapStr s)
{
    if (!s)
        return;

    ResultUtf8Iter iterRes = utf8_iter_str(s);
    if (iterRes.error.code != ERR_OK)
        return;

    Utf8Iter it = iterRes.value;

    while (utf8_iter_has_next(&it))
    {
        i8 *current = (i8 *)it.ptr;
        ResultUtf8Codepoint cpRes = utf8_iter_next(&it);

        if (cpRes.error.code != ERR_OK)
            return;

        if (cpRes.value.codepoint <= 0x7Fu)
        current[0] = char_to_lower_ascii(current[0]);
    }
}

/**
 * @brief Converts a UTF-8 string to uppercase in place (ASCII only).
 * Modifies the provided buffer directly.
 *
 * ```c
 * HeapStr text = ConstToHeapStr(default_allocator(), "caf\xC3\xA9");
 * string_to_upper_inplace(text);
 * // text == "CAF\xC3\x89"
 * ```
 * @param s Mutable UTF-8 string buffer
 */
static inline void string_to_upper_inplace(HeapStr s)
{
    if (!s)
        return;

    ResultUtf8Iter iterRes = utf8_iter_str(s);
    if (iterRes.error.code != ERR_OK)
        return;

    Utf8Iter it = iterRes.value;

    while (utf8_iter_has_next(&it))
    {
        i8 *current = (i8 *)it.ptr;
        ResultUtf8Codepoint cpRes = utf8_iter_next(&it);

        if (cpRes.error.code != ERR_OK)
            return;

        if (cpRes.value.codepoint <= 0x7Fu)
            current[0] = char_to_upper_ascii(current[0]);
    }
}

/**
 * @brief Performs a in-place replacement of lowercase characters with uppercase ones.
 * Provided string MUST be modifiable and non const.
 *
 * @param s
 */
static inline void string_to_upper_inplace_ascii(HeapStr s)
{
    if (!s)
        return;

    u64 i = 0;
    while (s[i])
    {
        s[i] = char_to_upper_ascii(s[i]);
        ++i;
    }
}

/**
 * @brief Returns a copy of `s` with all uppercase characters replaced with lowercase ones.
 *
 * @param alloc
 * @param s
 * @return ResultOwnedStr
 */
static inline ResultOwnedStr string_lower_ascii(Allocator *a, ConstStr s)
{
    if (!a || !s)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "string_lower_ascii", ERR_INVALID_PARAMETER, "null arg"),
        };

    const u64 len = string_size(s);
    HeapStr copy = (HeapStr)a->alloc(a, len + 1);
    if (!copy)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "string_lower_ascii", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    for (u64 i = 0; i < len; ++i)
        copy[i] = char_to_lower_ascii(s[i]);
    copy[len] = 0;

    return (ResultOwnedStr){
        .value = copy,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Creates a lowercase copy of a UTF-8 string (ASCII fold).
 * Memory is owned by the caller and must be released with the same allocator.
 *
 * ```c
 * ResultOwnedStr res = string_lower(default_allocator(), "CAF\xC3\x89");
 * if (res.error.code) // Handle error
 * // res.value == "caf\xC3\xA9"
 * ```
 * @param a Allocator for the resulting string
 * @param s UTF-8 encoded source string
 * @return ResultOwnedStr
 */
static inline ResultOwnedStr string_lower(Allocator *a, ConstStr s)
{
    ResultOwnedStr copyRes = string_dupe(a, s);
    if (copyRes.error.code != ERR_OK)
        return copyRes;

    string_to_lower_inplace(copyRes.value);
    return copyRes;
}

/**
 * @brief Returns a copy of `s` with all lowercase characters replaced with uppercase ones.
 *
 * @param alloc
 * @param s
 * @return ResultOwnedStr
 */
static inline ResultOwnedStr string_upper_ascii(Allocator *a, ConstStr s)
{
    if (!a || !s)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "string_upper_ascii", ERR_INVALID_PARAMETER, "null arg"),
        };

    const u64 len = string_size(s);
    HeapStr copy = (HeapStr)a->alloc(a, len + 1);
    if (!copy)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "string_upper_ascii", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    for (u64 i = 0; i < len; ++i)
        copy[i] = char_to_upper_ascii(s[i]);
    copy[len] = 0;

    return (ResultOwnedStr){
        .value = copy,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Creates an uppercase copy of a UTF-8 string (ASCII fold).
 * Memory is owned by the caller and must be released with the same allocator.
 *
 * ```c
 * ResultOwnedStr res = string_upper(default_allocator(), "caf\xC3\xA9");
 * if (res.error.code) // Handle error
 * // res.value == "CAF\xC3\x89"
 * ```
 * @param a Allocator for the resulting string
 * @param s UTF-8 encoded source string
 * @return ResultOwnedStr
 */
static inline ResultOwnedStr string_upper(Allocator *a, ConstStr s)
{
    ResultOwnedStr copyRes = string_dupe(a, s);
    if (copyRes.error.code != ERR_OK)
        return copyRes;

    string_to_upper_inplace(copyRes.value);
    return copyRes;
}

/**
 * @brief Trims whitespace characters from the start and/or end of a string.
 * Creates a new string with the trimmed result.
 *
 * Memory is owned by the caller and should be freed using the same allocator.
 *
 * ```c
 * ConstStr myStr = "  Hello, World!  ";
 * ResultOwnedStr trimmed = string_trim_whitespace_ascii(default_allocator(), myStr, true, true);
 * if (trimmed.error.code) // Error!
 * // trimmed.value == "Hello, World!"
 *
 * ConstStr myStr2 = "  Hello, World!  ";
 * ResultOwnedStr trimStart = string_trim_whitespace_ascii(default_allocator(), myStr2, true, false);
 * if (trimStart.error.code) // Error!
 * // trimStart.value == "Hello, World!  "
 * ```
 *
 * @param alloc Allocator to use for the new string
 * @param s String to trim
 * @param start If !0 (true), trim from the start of the string
 * @param end If !0 (true), trim from the end of the string
 * @return ResultOwnedStr
 */
static inline ResultOwnedStr string_trim_whitespace_ascii(Allocator *a, ConstStr s, ibool start, ibool end)
{
    if (!a || !s)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("string", "string_trim_whitespace_ascii", ERR_INVALID_PARAMETER, "null arg"),
        };

    u64 len = string_size(s);
    u64 startIdx = 0;
    u64 endIdx = len;

    if (start)
    {
        while (startIdx < len && char_is_whitespace_ascii(s[startIdx]))
        {
            ++startIdx;
        }
    }

    if (end)
    {
        while (endIdx > startIdx && char_is_whitespace_ascii(s[endIdx - 1]))
        {
            --endIdx;
        }
    }

    if (startIdx >= endIdx)
    {
        HeapStr newStr = string_dupe_noresult(a, "");

        if (!newStr)
            return (ResultOwnedStr){
                .error = X_ERR_EXT("string", "string_trim_whitespace_ascii", ERR_OUT_OF_MEMORY, "alloc failure"),
            };

        return (ResultOwnedStr){
            .value = newStr,
            .error = X_ERR_OK,
        };
    }

    u64 newLen = endIdx - startIdx;
    HeapStr newStr = (HeapStr)a->alloc(a, newLen + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("string", "string_trim_whitespace_ascii", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    string_copy_n_unsafe(s + startIdx, newStr, newLen, false);
    newStr[newLen] = (i8)0;

    return (ResultOwnedStr){
        .value = newStr,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Trims Unicode whitespace from both ends of a UTF-8 string.
 * Memory is owned by the caller and must be released with the same allocator.
 *
 * ```c
 * ConstStr text = " \tHello! \xE2\x80\x83";
 * ResultOwnedStr res = string_trim_whitespace(default_allocator(), text, true, true);
 * if (res.error.code) // Handle error
 * // res.value == "Hello!"
 * ```
 * @param a Allocator used for the resulting string
 * @param s UTF-8 encoded source string
 * @param start Trim leading whitespace if true
 * @param end Trim trailing whitespace if true
 * @return ResultOwnedStr
 */
static inline ResultOwnedStr string_trim_whitespace(Allocator *a, ConstStr s, ibool start, ibool end)
{
    if (!a || !s)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("string", "string_trim_whitespace", ERR_INVALID_PARAMETER, "null arg"),
        };

    ResultUtf8Iter iterRes = utf8_iter_str(s);
    if (iterRes.error.code != ERR_OK)
        return (ResultOwnedStr){
            .error = iterRes.error,
        };

    Utf8Iter startIt = iterRes.value;

    if (start)
    {
        while (utf8_iter_has_next(&startIt))
        {
            ResultUtf8Codepoint cpRes = utf8_iter_peek(&startIt);
            if (cpRes.error.code != ERR_OK)
                return (ResultOwnedStr){
                    .error = cpRes.error,
                };

            if (!_utf8_is_whitespace_cp(cpRes.value.codepoint))
                break;

            utf8_iter_next(&startIt);
        }
    }

    Utf8Iter scanIt = startIt;
    ConstStr endPtr = startIt.ptr;

    while (utf8_iter_has_next(&scanIt))
    {
        ResultUtf8Codepoint cpRes = utf8_iter_next(&scanIt);
        if (cpRes.error.code != ERR_OK)
            return (ResultOwnedStr){
                .error = cpRes.error,
            };

        if (!end || !_utf8_is_whitespace_cp(cpRes.value.codepoint))
            endPtr = scanIt.ptr;
    }

    u64 byteLen = (u64)(endPtr - startIt.ptr);

    if (byteLen == 0)
    {
        HeapStr newStr = string_dupe_noresult(a, "");
        if (!newStr)
            return (ResultOwnedStr){
                .error = X_ERR_EXT("string", "string_trim_whitespace", ERR_OUT_OF_MEMORY, "alloc failure"),
            };

        return (ResultOwnedStr){
            .value = newStr,
            .error = X_ERR_OK,
        };
    }

    HeapStr newStr = (HeapStr)a->alloc(a, byteLen + 1);
    if (!newStr)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("string", "string_trim_whitespace", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    string_copy_n_unsafe(startIt.ptr, newStr, byteLen, false);
    newStr[byteLen] = 0;

    return (ResultOwnedStr){
        .value = newStr,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Rerturns a character version of an integer within the range 0-9
 * For example: (i16)0 returns '0'
 *
 * @param i
 * @return i8
 */
static inline i8 digit_to_char(const i16 i)
{
    if (i > 9)
        return 0;

    return "0123456789"[i];
}

/**
 * @brief Stringifies an integer.
 *
 * @param alloc
 * @param i
 * @return ResultOwnedStr
 */
static inline ResultOwnedStr string_from_int(Allocator *a, const i64 i)
{
    char buf[20];
    i64 n = i;
    i16 idx = 18;

    if (n == 0)
    {
        HeapStr intStr = (HeapStr)a->alloc(a, 2);

        if (!intStr)
            return (ResultOwnedStr){
                .error = X_ERR_EXT("string", "string_from_int", ERR_OUT_OF_MEMORY, "alloc failure"),
            };

        intStr[0] = '0';
        intStr[1] = '\0';
        return (ResultOwnedStr){
            .value = intStr,
            .error = X_ERR_OK,
        };
    }

    if (i < 0)
    {
        n = -n;
    }

    while (n != 0)
    {
        i16 d = (i16)(n % 10);
        buf[idx] = digit_to_char(d);
        --idx;
        n /= 10;
    }

    if (i < 0)
    {
        buf[idx] = '-';
        --idx;
    }

    buf[19] = '\0';

    HeapStr intStr = (HeapStr)a->alloc(a, (19 - (u64)idx) + 1);
    if (!intStr)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("string", "string_from_int", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    string_copy_unsafe(buf + idx + 1, intStr);

    return (ResultOwnedStr){
        .value = intStr,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Stringifies a unsigned integer.
 *
 * @param alloc
 * @param i
 * @return ResultOwnedStr
 */
static inline ResultOwnedStr string_from_uint(Allocator *a, const u64 i)
{
    char buf[20];
    u64 n = i;
    i16 idx = 18;

    if (n == 0)
    {
        HeapStr intStr = (HeapStr)a->alloc(a, 2);

        if (!intStr)
            return (ResultOwnedStr){
                .error = X_ERR_EXT("string", "string_from_uint", ERR_OUT_OF_MEMORY, "alloc failure"),
            };

        intStr[0] = '0';
        intStr[1] = '\0';
        return (ResultOwnedStr){
            .value = intStr,
            .error = X_ERR_OK,
        };
    }

    while (n != 0)
    {
        i16 d = (i16)(n % 10);
        buf[idx] = digit_to_char(d);
        --idx;
        n /= 10;
    }

    buf[19] = '\0';

    HeapStr intStr = (HeapStr)a->alloc(a, (19 - (u64)idx) + 1);

    if (!intStr)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("string", "string_from_uint", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    string_copy_unsafe(buf + idx + 1, intStr);

    return (ResultOwnedStr){
        .value = intStr,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Stringifies a float.
 *
 * @param alloc
 * @param flt
 * @param precision Must be in range 0-18
 * @return ResultOwnedStr
 */
static inline ResultOwnedStr string_from_float(Allocator *a, const f64 flt, const u64 precision)
{
    if (precision == 0)
        return string_from_int(a, (i64)flt);
    
    if (precision > 18)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("string", "string_from_float",
                ERR_INVALID_PARAMETER, "precision > 18"),
        };

    f64 absVal = (flt >= 0.0) ? flt : -flt;
    i64 intPart = (i64)absVal;
    f64 fracPart = absVal - (f64)intPart;

    f64 scale = math_f64_power(10.0, precision);
    u64 fracInt = (u64)(fracPart * scale + 0.5); // rounding

    if (fracInt >= (u64)scale)
    {
        fracInt -= (u64)scale;
        ++intPart;
    }

    i64 signedIntPart = (flt < 0.0) ? -intPart : intPart;

    ResultOwnedStr intStrRes = string_from_int(a, signedIntPart);
    if (intStrRes.error.code != ERR_OK)
        return intStrRes;

    ResultOwnedStr fracStrRes = string_from_uint(a, fracInt);
    if (fracStrRes.error.code != ERR_OK)
    {
        a->free(a, intStrRes.value);
        return fracStrRes;
    }

    u64 intLen = string_size(intStrRes.value);
    u64 fracLen = string_size(fracStrRes.value);
    u64 padLen = (precision > fracLen) ? (precision - fracLen) : 0;
    u64 totalLen = intLen + 1 + precision;

    HeapStr finalStr = (HeapStr)a->alloc(a, totalLen + 1);
    if (!finalStr)
    {
        a->free(a, intStrRes.value);
        a->free(a, fracStrRes.value);
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("string", "string_from_float", ERR_OUT_OF_MEMORY, "alloc failure"),
        };
    }

    string_copy_n_unsafe(intStrRes.value, finalStr, intLen, false);
    finalStr[intLen] = '.';

    u64 cursor = intLen + 1;
    for (u64 i = 0; i < padLen; ++i)
        finalStr[cursor++] = '0';

    string_copy_n_unsafe(fracStrRes.value, finalStr + cursor, fracLen, false);
    cursor += fracLen;

    while (cursor < intLen + 1 + precision)
        finalStr[cursor++] = '0';

    finalStr[cursor] = 0;

    a->free(a, intStrRes.value);
    a->free(a, fracStrRes.value);

    return (ResultOwnedStr){
        .value = finalStr,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Parses a positive or negative integer from a provided string.
 *
 * @param s
 * @return ResultI64
 */
static inline ResultI64 string_parse_int_ascii(ConstStr s)
{
    if (!s || *s == '\0')
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("string", "string_parse_int_ascii", ERR_INVALID_PARAMETER, "null or empty str"),
        };

    const i8 *ptr = s;
    i64 result = 0;
    i64 sign = 1;

    while (char_is_whitespace_ascii(*ptr))
        ++ptr;

    if (*ptr == '-' || *ptr == '+')
    {
        if (*ptr == '-')
            sign = -1;
        ++ptr;
    }

    if (!char_is_digit_ascii(*ptr))
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("string", "string_parse_int_ascii", ERR_UNEXPECTED_BYTE, "byte not digit"),
        };

    while (char_is_digit_ascii(*ptr))
    {
        i64 newVal = result * 10 + (*ptr - '0');

        if (newVal < result)
            return (ResultI64){
                .value = 0,
            .error = X_ERR_EXT("string", "string_parse_int_ascii", ERR_WOULD_OVERFLOW, "integer overflow"),
            };

        result = newVal;
        ++ptr;
    }

    if (*ptr != '\0' && !char_is_whitespace_ascii(*ptr))
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("string", "string_parse_int_ascii", ERR_UNEXPECTED_BYTE, "byte not digit"),
        };

    return (ResultI64){
        .value = result * sign,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Parses a signed 64-bit integer from a UTF-8 string (ASCII digits only).
 *
 * ```c
 * ResultI64 res = string_parse_int("  -42 ");
 * if (res.error.code) // Handle error
 * // res.value == -42
 * ```
 * @param s UTF-8 encoded numeric string
 * @return ResultI64
 */
static inline ResultI64 string_parse_int(ConstStr s)
{
    if (!s || *s == '\0')
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("string", "string_parse_int", ERR_INVALID_PARAMETER, "null or empty str"),
        };

    ResultUtf8Iter iterRes = utf8_iter_str(s);
    if (iterRes.error.code != ERR_OK)
        return (ResultI64){
            .value = 0,
            .error = iterRes.error,
        };

    Utf8Iter it = iterRes.value;

    while (utf8_iter_has_next(&it))
    {
        ResultUtf8Codepoint peekRes = utf8_iter_peek(&it);
        if (peekRes.error.code != ERR_OK)
            return (ResultI64){
                .value = 0,
                .error = peekRes.error,
            };

        if (!_utf8_is_whitespace_cp(peekRes.value.codepoint))
            break;

        utf8_iter_next(&it);
    }

    if (!utf8_iter_has_next(&it))
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("string", "string_parse_int", ERR_UNEXPECTED_BYTE, "byte not digit"),
        };

    i64 sign = 1;
    ResultUtf8Codepoint cpRes = utf8_iter_peek(&it);
    if (cpRes.error.code != ERR_OK)
        return (ResultI64){
            .value = 0,
            .error = cpRes.error,
        };

    if (cpRes.value.codepoint == '-' || cpRes.value.codepoint == '+')
    {
        if (cpRes.value.codepoint == '-')
            sign = -1;
        utf8_iter_next(&it);
    }

    if (!utf8_iter_has_next(&it))
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("string", "string_parse_int", ERR_UNEXPECTED_BYTE, "byte not digit"),
        };

    cpRes = utf8_iter_peek(&it);
    if (cpRes.error.code != ERR_OK)
        return (ResultI64){
            .value = 0,
            .error = cpRes.error,
        };

    if (!_utf8_is_ascii_digit(cpRes.value.codepoint))
        return (ResultI64){
            .value = 0,
            .error = X_ERR_EXT("string", "string_parse_int", ERR_UNEXPECTED_BYTE, "byte not digit"),
        };

    i64 result = 0;

    while (utf8_iter_has_next(&it))
    {
        ResultUtf8Codepoint digitRes = utf8_iter_peek(&it);
        if (digitRes.error.code != ERR_OK)
            return (ResultI64){
                .value = 0,
                .error = digitRes.error,
            };

        if (!_utf8_is_ascii_digit(digitRes.value.codepoint))
            break;

        ResultUtf8Codepoint advanceRes = utf8_iter_next(&it);
        if (advanceRes.error.code != ERR_OK)
            return (ResultI64){
                .value = 0,
                .error = advanceRes.error,
            };

        i64 newVal = result * 10 + (i64)(advanceRes.value.codepoint - '0');
        if (newVal < result)
            return (ResultI64){
                .value = 0,
                .error = X_ERR_EXT("string", "string_parse_int", ERR_WOULD_OVERFLOW, "integer overflow"),
            };

        result = newVal;
    }

    while (utf8_iter_has_next(&it))
    {
        cpRes = utf8_iter_peek(&it);
        if (cpRes.error.code != ERR_OK)
            return (ResultI64){
                .value = 0,
                .error = cpRes.error,
            };

        if (!_utf8_is_whitespace_cp(cpRes.value.codepoint))
            return (ResultI64){
                .value = 0,
                .error = X_ERR_EXT("string", "string_parse_int", ERR_UNEXPECTED_BYTE, "byte not digit"),
            };

        utf8_iter_next(&it);
    }

    return (ResultI64){
        .value = result * sign,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Parses a unsigned integer from a provided string.
 *
 * @param s
 * @return ResultI64
 */
static inline ResultU64 string_parse_uint_ascii(ConstStr s)
{
    if (!s || *s == '\0')
        return (ResultU64){
            .value = 0,
            .error = X_ERR_EXT("string", "string_parse_uint_ascii", ERR_INVALID_PARAMETER, "null or empty str"),
        };

    const i8 *ptr = s;
    u64 result = 0;

    while (char_is_whitespace_ascii(*ptr))
        ++ptr;

    if (!char_is_digit_ascii(*ptr))
        return (ResultU64){
            .value = 0,
            .error = X_ERR_EXT("string", "string_parse_uint_ascii", ERR_UNEXPECTED_BYTE, "byte not digit"),
        };

    while (char_is_digit_ascii(*ptr))
    {
        u64 newVal = result * 10 + (u64)(*ptr - '0');

        if (newVal < result)
            return (ResultU64){
                .value = 0,
            .error = X_ERR_EXT("string", "string_parse_uint_ascii", ERR_WOULD_OVERFLOW, "integer overflow"),
            };

        result = newVal;
        ++ptr;
    }

    if (*ptr != '\0' && !char_is_whitespace_ascii(*ptr))
        return (ResultU64){
            .value = 0,
            .error = X_ERR_EXT("string", "string_parse_uint_ascii", ERR_UNEXPECTED_BYTE, "byte not digit"),
        };

    return (ResultU64){
        .value = result,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Parses an unsigned 64-bit integer from a UTF-8 string (ASCII digits only).
 *
 * ```c
 * ResultU64 res = string_parse_uint(" 1024 ");
 * if (res.error.code) // Handle error
 * // res.value == 1024
 * ```
 * @param s UTF-8 encoded numeric string
 * @return ResultU64
 */
static inline ResultU64 string_parse_uint(ConstStr s)
{
    if (!s || *s == '\0')
        return (ResultU64){
            .value = 0,
            .error = X_ERR_EXT("string", "string_parse_uint", ERR_INVALID_PARAMETER, "null or empty str"),
        };

    ResultUtf8Iter iterRes = utf8_iter_str(s);
    if (iterRes.error.code != ERR_OK)
        return (ResultU64){
            .value = 0,
            .error = iterRes.error,
        };

    Utf8Iter it = iterRes.value;

    while (utf8_iter_has_next(&it))
    {
        ResultUtf8Codepoint peekRes = utf8_iter_peek(&it);
        if (peekRes.error.code != ERR_OK)
            return (ResultU64){
                .value = 0,
                .error = peekRes.error,
            };

        if (!_utf8_is_whitespace_cp(peekRes.value.codepoint))
            break;

        utf8_iter_next(&it);
    }

    if (!utf8_iter_has_next(&it))
        return (ResultU64){
            .value = 0,
            .error = X_ERR_EXT("string", "string_parse_uint", ERR_UNEXPECTED_BYTE, "byte not digit"),
        };

    ResultUtf8Codepoint cpRes = utf8_iter_peek(&it);
    if (cpRes.error.code != ERR_OK)
        return (ResultU64){
            .value = 0,
            .error = cpRes.error,
        };

    if (!_utf8_is_ascii_digit(cpRes.value.codepoint))
        return (ResultU64){
            .value = 0,
            .error = X_ERR_EXT("string", "string_parse_uint", ERR_UNEXPECTED_BYTE, "byte not digit"),
        };

    u64 result = 0;

    while (utf8_iter_has_next(&it))
    {
        ResultUtf8Codepoint digitRes = utf8_iter_peek(&it);
        if (digitRes.error.code != ERR_OK)
            return (ResultU64){
                .value = 0,
                .error = digitRes.error,
            };

        if (!_utf8_is_ascii_digit(digitRes.value.codepoint))
            break;

        ResultUtf8Codepoint advanceRes = utf8_iter_next(&it);
        if (advanceRes.error.code != ERR_OK)
            return (ResultU64){
                .value = 0,
                .error = advanceRes.error,
            };

        u64 newVal = result * 10 + (u64)(advanceRes.value.codepoint - '0');
        if (newVal < result)
            return (ResultU64){
                .value = 0,
                .error = X_ERR_EXT("string", "string_parse_uint", ERR_WOULD_OVERFLOW, "integer overflow"),
            };

        result = newVal;
    }

    while (utf8_iter_has_next(&it))
    {
        cpRes = utf8_iter_peek(&it);
        if (cpRes.error.code != ERR_OK)
            return (ResultU64){
                .value = 0,
                .error = cpRes.error,
            };

        if (!_utf8_is_whitespace_cp(cpRes.value.codepoint))
            return (ResultU64){
                .value = 0,
                .error = X_ERR_EXT("string", "string_parse_uint", ERR_UNEXPECTED_BYTE, "byte not digit"),
            };

        utf8_iter_next(&it);
    }

    return (ResultU64){
        .value = result,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Parses a float from a provided string.
 *
 * @param s
 * @return ResultF64
 */
static inline ResultF64 string_parse_float_ascii(ConstStr s)
{
    if (!s || *s == '\0')
        return (ResultF64){
            .value = 0.0,
            .error = X_ERR_EXT("string", "string_parse_float_ascii", ERR_INVALID_PARAMETER, "null or empty str"),
        };

    const i8 *ptr = s;
    f64 result = 0.0;
    f64 sign = 1.0;

    while (char_is_whitespace_ascii(*ptr))
        ++ptr;

    if (*ptr == '-' || *ptr == '+')
    {
        if (*ptr == '-')
            sign = -1.0;
        ++ptr;
    }

    if (!char_is_digit_ascii(*ptr) && *ptr != '.')
        return (ResultF64){
            .value = 0.0,
            .error = X_ERR_EXT("string", "string_parse_float_ascii", ERR_UNEXPECTED_BYTE, "byte not digit"),
        };

    while (char_is_digit_ascii(*ptr))
    {
        f64 newVal = result * (f64)10.0 + (f64)(*ptr - '0');

        if (newVal < result)
            return (ResultF64){
                .value = 0.0,
                .error = X_ERR_EXT("string", "string_parse_float_ascii", ERR_WOULD_OVERFLOW, "integer overflow"),
            };

        result = newVal;
        ++ptr;
    }

    if (*ptr == '.')
    {
        ++ptr;
        f64 fraction = 0.0;
        f64 divisor = 10.0;

        if (!char_is_digit_ascii(*ptr))
            return (ResultF64){
                .value = 0.0,
                .error = X_ERR_EXT("string", "string_parse_uint_ascii", ERR_UNEXPECTED_BYTE, "byte not digit"),
            };

        while (char_is_digit_ascii(*ptr))
        {
            f64 newFrac = fraction + (*ptr - '0') / divisor;

            if (newFrac < fraction)
                return (ResultF64){
                    .value = 0.0,
                    .error = X_ERR_EXT("string", "string_parse_float_ascii", ERR_WOULD_OVERFLOW, "integer overflow"),
                };

            fraction = newFrac;
            divisor *= (f64)10.0;
            ++ptr;
        }
        result += fraction;
    }

    if (*ptr != '\0' && !char_is_whitespace_ascii(*ptr))
        return (ResultF64){
            .value = 0.0,
            .error = X_ERR_EXT("string", "string_parse_uint_ascii", ERR_UNEXPECTED_BYTE, "byte not digit"),
        };

    return (ResultF64){
        .value = result * sign,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Parses a double-precision floating point number from a UTF-8 string (ASCII digits only).
 *
 * ```c
 * ResultF64 res = string_parse_float("\t3.1415 ");
 * if (res.error.code) // Handle error
 * // res.value == 3.1415
 * ```
 * @param s UTF-8 encoded numeric string
 * @return ResultF64
 */
static inline ResultF64 string_parse_float(ConstStr s)
{
    if (!s || *s == '\0')
        return (ResultF64){
            .value = 0.0,
            .error = X_ERR_EXT("string", "string_parse_float", ERR_INVALID_PARAMETER, "null or empty str"),
        };

    ResultUtf8Iter iterRes = utf8_iter_str(s);
    if (iterRes.error.code != ERR_OK)
        return (ResultF64){
            .value = 0.0,
            .error = iterRes.error,
        };

    Utf8Iter it = iterRes.value;

    while (utf8_iter_has_next(&it))
    {
        ResultUtf8Codepoint peekRes = utf8_iter_peek(&it);
        if (peekRes.error.code != ERR_OK)
            return (ResultF64){
                .value = 0.0,
                .error = peekRes.error,
            };

        if (!_utf8_is_whitespace_cp(peekRes.value.codepoint))
            break;

        utf8_iter_next(&it);
    }

    if (!utf8_iter_has_next(&it))
        return (ResultF64){
            .value = 0.0,
            .error = X_ERR_EXT("string", "string_parse_float", ERR_UNEXPECTED_BYTE, "byte not digit"),
        };

    f64 sign = 1.0;
    ResultUtf8Codepoint cpRes = utf8_iter_peek(&it);
    if (cpRes.error.code != ERR_OK)
        return (ResultF64){
            .value = 0.0,
            .error = cpRes.error,
        };

    if (cpRes.value.codepoint == '-' || cpRes.value.codepoint == '+')
    {
        if (cpRes.value.codepoint == '-')
            sign = -1.0;
        utf8_iter_next(&it);
    }

    f64 result = 0.0;
    ibool sawDigit = false;

    while (utf8_iter_has_next(&it))
    {
        cpRes = utf8_iter_peek(&it);
        if (cpRes.error.code != ERR_OK)
            return (ResultF64){
                .value = 0.0,
                .error = cpRes.error,
            };

        if (!_utf8_is_ascii_digit(cpRes.value.codepoint))
            break;

        ResultUtf8Codepoint advanceRes = utf8_iter_next(&it);
        if (advanceRes.error.code != ERR_OK)
            return (ResultF64){
                .value = 0.0,
                .error = advanceRes.error,
            };

        f64 newVal = result * 10.0 + (f64)(advanceRes.value.codepoint - '0');
        if (newVal < result)
            return (ResultF64){
                .value = 0.0,
                .error = X_ERR_EXT("string", "string_parse_float", ERR_WOULD_OVERFLOW, "integer overflow"),
            };

        result = newVal;
        sawDigit = true;
    }

    if (utf8_iter_has_next(&it))
    {
        cpRes = utf8_iter_peek(&it);
        if (cpRes.error.code != ERR_OK)
            return (ResultF64){
                .value = 0.0,
                .error = cpRes.error,
            };

        if (cpRes.value.codepoint == '.')
        {
            utf8_iter_next(&it);

            ResultUtf8Codepoint fracPeek = utf8_iter_peek(&it);
            if (fracPeek.error.code != ERR_OK)
                return (ResultF64){
                    .value = 0.0,
                    .error = fracPeek.error,
                };

            if (!_utf8_is_ascii_digit(fracPeek.value.codepoint))
                return (ResultF64){
                    .value = 0.0,
                    .error = X_ERR_EXT("string", "string_parse_float", ERR_UNEXPECTED_BYTE, "byte not digit"),
                };

            f64 fraction = 0.0;
            f64 divisor = 10.0;

            while (utf8_iter_has_next(&it))
            {
                ResultUtf8Codepoint digitRes = utf8_iter_peek(&it);
                if (digitRes.error.code != ERR_OK)
                    return (ResultF64){
                        .value = 0.0,
                        .error = digitRes.error,
                    };

                if (!_utf8_is_ascii_digit(digitRes.value.codepoint))
                    break;

                ResultUtf8Codepoint advanceRes = utf8_iter_next(&it);
                if (advanceRes.error.code != ERR_OK)
                    return (ResultF64){
                        .value = 0.0,
                        .error = advanceRes.error,
                    };

                f64 newFrac = fraction + (advanceRes.value.codepoint - '0') / divisor;
                if (newFrac < fraction)
                    return (ResultF64){
                        .value = 0.0,
                        .error = X_ERR_EXT("string", "string_parse_float", ERR_WOULD_OVERFLOW, "integer overflow"),
                    };

                fraction = newFrac;
                divisor *= 10.0;
                sawDigit = true;
            }

            result += fraction;
        }
    }

    if (!sawDigit)
        return (ResultF64){
            .value = 0.0,
            .error = X_ERR_EXT("string", "string_parse_float", ERR_UNEXPECTED_BYTE, "byte not digit"),
        };

    while (utf8_iter_has_next(&it))
    {
        cpRes = utf8_iter_peek(&it);
        if (cpRes.error.code != ERR_OK)
            return (ResultF64){
                .value = 0.0,
                .error = cpRes.error,
            };

        if (!_utf8_is_whitespace_cp(cpRes.value.codepoint))
            return (ResultF64){
                .value = 0.0,
                .error = X_ERR_EXT("string", "string_parse_float", ERR_UNEXPECTED_BYTE, "byte not digit"),
            };

        utf8_iter_next(&it);
    }

    return (ResultF64){
        .value = result * sign,
        .error = X_ERR_OK,
    };
}
