#pragma once

#include "xstd_core.h"
#include "xstd_error.h"
#include "xstd_alloc.h"
#include "xstd_result.h"
#include "xstd_list.h"

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
u64 string_size(ConstStr s)
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
ibool string_equals(ConstStr a, ConstStr b)
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
 * ResultOwnedStr myString = string_alloc(&c_allocator, 3, 'w');
 * if (myString.error) // Error!
 * // myString.value == ['w', 'w', 'w', 0(end sentinel)]
 * ```
 * @param alloc
 * @param sizeNonTerminated
 * @param fill
 * @return ResultOwnedStr
 */
ResultOwnedStr string_alloc(Allocator *alloc, u64 sizeNonTerminated, i8 fill)
{
    HeapStr str = alloc->alloc(alloc, sizeNonTerminated + 1);

    if (!str)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_OUT_OF_MEMORY,
        };

    u64 i = sizeNonTerminated + 1;

    while (i--)
    {
        str[i] = fill;
    }
    str[sizeNonTerminated] = (i8)0;

    return (ResultOwnedStr){
        .value = str,
        .error = ERR_OK,
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
 * String dest = ConstToStr("____________________");
 * Error err = string_copy(source, dest);
 * if (err) // Error!
 * // dest == "Example string"
 * ```
 * @param source Terminated string
 * @param destination String of size >= source size
 * @return Error
 */
Error string_copy(ConstStr source, String destination)
{
    if (!source || !destination)
        return ERR_INVALID_PARAMETER;

    u64 srcLen = string_size(source);
    u64 destLen = string_size(destination);

    if (destLen < srcLen)
        return ERR_WOULD_OVERFLOW;

    u64 i = 0;
    while (source[i])
    {
        destination[i] = source[i];
        ++i;
    }
    destination[i] = (i8)0;

    return ERR_OK;
}

/**
 * @brief Copies `n` characters from `source` to `destination`. Checks if the `destination`
 * is big enough to receive n characters, if not returns `ERR_WOULD_OVERFLOW`.
 *
 * ```c
 * ConstStr source = "Example string";
 * String dest = ConstToStr("______________");
 * Error err = string_copy_n(source, dest, 5, false);
 * if (err) // Error!
 * // dest == "Examp_________"
 * ```
 * @warning DOES NOT TERMINATE `destination` STRING
 * @param source Terminated string
 * @param destination String of size >= n
 * @param terminate if the destination buffer should be NULL terminated after the copied characters.
 * @return Error
 */
Error string_copy_n(ConstStr source, String destination, u64 n, ibool terminate)
{
    if (!source || !destination)
        return ERR_INVALID_PARAMETER;

    u64 srcLen = string_size(source);
    u64 destLen = string_size(destination);

    if (srcLen < n)
        return ERR_WOULD_OVERFLOW;

    if (destLen < n)
        return ERR_WOULD_OVERFLOW;

    u64 i = 0;
    while (i < n)
    {
        destination[i] = source[i];
        ++i;
    }

    if (terminate)
        destination[i] = (i8)0;

    return ERR_OK;
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
 * String dest = ConstToStr("__________________");
 * string_copy_unsafe(source, dest);
 * // dest == "Example string"
 * ```
 * @param source Terminated string of size >= n
 * @param destination Terminated string of size >= n
 */
void string_copy_unsafe(ConstStr source, String destination)
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
 * String dest = ConstToStr("______________");
 * string_copy_n_unsafe(source, dest, 5, false);
 * // dest == "Examp_________"
 * ```
 * @param source Terminated string of size >= n
 * @param destination Terminated string of size >= n
 * @param terminate if the destination buffer should be NULL terminated after the copied characters.
 */
void string_copy_n_unsafe(ConstStr source, String destination, u64 n, ibool terminate)
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
 * ResultOwnedStr newString = string_dupe(&c_allocator, source);
 * if (newString.error) // Error!
 * // newString.value == "Example string"
 * ```
 * @param alloc
 * @param source Terminated string
 * @return String
 */
ResultOwnedStr string_dupe(Allocator *alloc, ConstStr source)
{
    if (!source)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_INVALID_PARAMETER,
        };

    u64 srcLen = string_size(source);
    HeapStr newStr = alloc->alloc(alloc, srcLen + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_OUT_OF_MEMORY,
        };

    string_copy_unsafe(source, newStr);

    return (ResultOwnedStr){
        .value = newStr,
        .error = ERR_OK,
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
 * String newString = string_dupe_noresult(&c_allocator, source);
 * if (!newString) // Error!
 * // newString == "Example string"
 * ```
 * @param alloc
 * @param source Terminated string
 * @return String
 */
HeapStr string_dupe_noresult(Allocator *alloc, ConstStr source)
{
    if (!source)
        return NULL;

    u64 srcLen = string_size(source);
    HeapStr newStr = alloc->alloc(alloc, srcLen + 1);

    if (!newStr)
        return NULL;

    string_copy_unsafe(source, newStr);
    return newStr;
}

// Converts a const char* to HeapStr, uses default allocator.
// TODO: take allocator instead of defaulting to c_alloc
#define ConstToHeapStr(constStr) string_dupe_noresult((Allocator *)&c_allocator, constStr)

/**
 * @brief Creates a copy of the string with a new size. Copies as much of the contents
 * of the original string back to the new allocated string and fills the rest with `fill`.
 * Always terminates the new string.
 *
 * Memory is owned by the caller and should be freed using the same allocator.
 *
 * ```c
 * ConstStr myString = "str";
 * ResultOwnedStr newStr = string_resize(&c_allocator, myString, 7, 'w');
 * if (newStr.error) // Error!
 * // newStr.value == ['s','t','r','w','w','w','w',0(end sentinel)]
 * ResultOwnedStr newStr2 = string_resize(&c_allocator, myString, 2, 'w');
 * if (newStr2.error) // Error!
 * // newStr2.value == ['s','t',0(end sentinel)]
 * ```
 * @param alloc
 * @param s Terminated string
 * @param newSizeNonTerminated Size of the new string
 * @param fill character to fill the empty allocated space
 */
ResultOwnedStr string_resize(Allocator *alloc, ConstStr source, u64 newSizeNonTerminated, i8 fill)
{
    if (!source)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_INVALID_PARAMETER,
        };

    u64 prevSize = string_size(source);
    u64 cpySize = prevSize < newSizeNonTerminated ? prevSize : newSizeNonTerminated;
    u64 fillCnt = newSizeNonTerminated - cpySize;
    HeapStr newStr = alloc->alloc(alloc, newSizeNonTerminated + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_OUT_OF_MEMORY,
        };

    string_copy_n_unsafe(source, newStr, cpySize, false);

    for (u64 i = 0; i < fillCnt; ++i)
    {
        newStr[cpySize + i] = fill;
    }
    newStr[newSizeNonTerminated] = (i8)0;

    return (ResultOwnedStr){
        .value = newStr,
        .error = ERR_OK,
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
 * ResultOwnedStr newStr = string_concat(&c_allocator, a, b);
 * if (newStr.error) // Error!
 * // newStr.value == "Hello, World!"
 * ```
 * @param alloc
 * @param a
 * @param b
 * @return String
 */
ResultOwnedStr string_concat(Allocator *alloc, ConstStr a, ConstStr b)
{
    if (!a || !b)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_INVALID_PARAMETER,
        };

    u64 aLen = string_size(a);
    u64 bLen = string_size(b);
    HeapStr newStr = alloc->alloc(alloc, aLen + bLen + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_OUT_OF_MEMORY,
        };

    string_copy_unsafe(a, newStr);
    string_copy_unsafe(b, newStr + aLen);

    return (ResultOwnedStr){
        .value = newStr,
        .error = ERR_OK,
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
 * ResultOwnedStr subStr = string_substr(&c_allocator, myStr, 7, 12);
 * if (subStr.error) // Error!
 * // subStr.value == "World"
 * ```
 * @param alloc
 * @param s
 * @param start
 * @param end
 * @return ResultOwnedStr
 */
ResultOwnedStr string_substr(Allocator *alloc, ConstStr s, u64 start, u64 end)
{
    if (!s || end < start)
        return (ResultOwnedStr){
            .error = ERR_INVALID_PARAMETER,
            .value = NULL,
        };

    u64 strSize = string_size(s);
    if (start > strSize || end > strSize)
        return (ResultOwnedStr){
            .error = ERR_INVALID_PARAMETER,
            .value = NULL,
        };

    u64 subSize = end - start;
    HeapStr newStr = alloc->alloc(alloc, subSize + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_OUT_OF_MEMORY,
        };

    string_copy_n_unsafe(s + start, newStr, subSize, false);
    newStr[subSize] = (i8)0;

    return (ResultOwnedStr){
        .value = newStr,
        .error = ERR_OK,
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
 * HeapStr subStr = string_substr_unsafe(&c_allocator, myStr, 7, 12);
 * if (!subStr) // Error!
 * // subStr == "World"
 * ```
 * @param alloc
 * @param s
 * @param start
 * @param end
 * @return HeapStr
 */
HeapStr string_substr_unsafe(Allocator *alloc, ConstStr s, u64 start, u64 end)
{
    u64 subSize = end - start;
    HeapStr newStr = alloc->alloc(alloc, subSize + 1);

    if (!newStr)
        return NULL;

    string_copy_n_unsafe(s + start, newStr, subSize, false);
    newStr[subSize] = (i8)0;
    return newStr;
}

/**
 * @brief Splits a string on every occurrence of `delimiter`.
 *
 * Memory of both strings and the list is owned by the caller and should be freed using the same allocator.
 *
 * ```c
 * ConstStr myStr = "Hello, World!";
 * ResultList split = string_split_char(&c_allocator, myStr, ' ');
 * if (split.error) // Error!
 * // split.value == List["Hello,", "World!"]
 * List l = res.value;
 * // Do list stuff
 * list_free_items(&c_allocator, &l); // Frees all allocated strings
 * list_deinit(&l); // Free the list
 * ```
 * @param alloc
 * @param s
 * @param delimiter
 * @return String
 */
ResultList string_split_char(Allocator *alloc, ConstStr s, i8 delimiter)
{
    // What a mess...

    if (!s)
        return (ResultList){
            .value = {0},
            .error = ERR_INVALID_PARAMETER,
        };

    ResultList resList = ListInitT(HeapStr, alloc);

    if (resList.error)
        return resList;

    List l = resList.value;

    u64 bound = string_size(s);
    u64 i = 0;
    u64 segStart = i;

    while (s[i])
    {
        if (s[i] == delimiter)
        {
            HeapStr subStr = string_substr_unsafe(alloc, s, segStart, i);

            if (!subStr)
                return (ResultList){
                    .value = {0},
                    .error = ERR_OUT_OF_MEMORY,
                };

            list_push(&l, &subStr);

            ++i;
            segStart = i;
            continue;
        }

        ++i;
    }

    HeapStr subStr = string_substr_unsafe(alloc, s, segStart, bound);

    if (!subStr)
        return (ResultList){
            .value = {0},
            .error = ERR_OUT_OF_MEMORY,
        };

    list_push(&l, &subStr);

    return (ResultList){
        .value = l,
        .error = ERR_OK,
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
i64 string_find_char(ConstStr haystack, i8 needle)
{
    if (!haystack || !needle)
        return -1;

    u64 i = 0;
    while (haystack[i])
    {
        if (haystack[i] == needle)
            return i;

        ++i;
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
i64 string_find(ConstStr haystack, ConstStr needle)
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
            return i;

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
 * if (resBuilder.error) // Error!
 * StringBuilder builder = resBuilder.value;
 * strbuilder_push_copy(&builder, "Hello,");
 * strbuilder_push_owned(&builder, ConstToHeapStr(" World!"));
 * ResultOwnedStr resBuiltStr = strbuilder_get_string(&builder);
 * if (resBuiltStr.error) // Error!
 * HeapStr builtStr = resBuiltStr.value;
 * strbuilder_deinit(&builder);
 * ```
 * @param alloc
 * @return ResultStrBuilder
 */
ResultStrBuilder strbuilder_init(Allocator *alloc)
{
    ResultList res = list_init(alloc, sizeof(HeapStr), 16);

    if (res.error)
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
        .error = ERR_OK,
    };
}

/**
 * @brief Clears the underlying data of the StringBuilder. Builder can be reused
 * after call to this function.
 *
 * @param builder
 */
void strbuilder_clear(StringBuilder *builder)
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
void strbuilder_deinit(StringBuilder *builder)
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
void strbuilder_push_owned(StringBuilder *builder, HeapStr s)
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
void strbuilder_push_copy(StringBuilder *builder, ConstStr s)
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
 * ResultStrBuilder res = strbuilder_init(&c_allocator);
 * if (res.error) // Error!
 * StringBuilder builder = res.value;
 * strbuilder_push_copy(&builder, "Hello,");
 * strbuilder_push_owned(&builder, ConstToHeapStr("World!"));
 * ResultOwnedStr built = strbuilder_get_string(&builder);
 * if (built.error) // Error!
 * HeapStr resultStr = built.value;
 * strbuilder_deinit(&builder);
 * ```
 * @param builder
 * @return ResultOwnedStr
 */
ResultOwnedStr strbuilder_get_string(StringBuilder *builder)
{
    if (!builder || !builder->_valid)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_INVALID_PARAMETER,
        };

    u64 totalSize = 0;
    u64 bound = list_size(&builder->_strings);

    for (u64 i = 0; i < bound; ++i)
    {
        HeapStr s = list_get_as_ptr(&builder->_strings, i);
        totalSize += string_size(s);
    }

    Allocator *alloc = &builder->_strings._allocator;
    HeapStr newStr = alloc->alloc(alloc, totalSize + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_OUT_OF_MEMORY,
        };

    u64 offset = 0;
    for (u64 i = 0; i < bound; ++i)
    {
        HeapStr s = list_get_as_ptr(&builder->_strings, i);
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
        .error = ERR_OK,
    };
}

/**
 * @brief Replaces every occurrences of `what` within `s` with `with`.
 *
 * Memory is owned by the caller and should be freed after use.
 *
 * ```c
 * ResultOwnedStr res = string_replace(&alloc, "This is a test, or is it?", "is", "is not");
 * if (res.error) // Error!
 * HeapStr replaced = res.value;
 * // replaced == "This is not a test, or is not it?"
 * ```
 * @param alloc
 * @param s
 * @param what
 * @param with
 * @return ResultOwnedStr
 */
ResultOwnedStr string_replace(Allocator *alloc, ConstStr s, ConstStr what, ConstStr with)
{
    if (!alloc || !s || !what || !with)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_INVALID_PARAMETER,
        };

    ResultStrBuilder strBldRes = strbuilder_init(alloc);

    if (strBldRes.error)
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
            HeapStr chunk = string_substr_unsafe(alloc, s + offset, 0, next);
            if (!chunk)
                return (ResultOwnedStr){
                    .value = NULL,
                    .error = ERR_OUT_OF_MEMORY,
                };

            strbuilder_push_owned(&strBld, chunk);

            HeapStr replaced = string_dupe_noresult(alloc, with);
            strbuilder_push_owned(&strBld, replaced);

            offset += next + whatSize;
        }
    }

    HeapStr tailChunk = string_dupe_noresult(alloc, s + offset);
    if (!tailChunk)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_OUT_OF_MEMORY,
        };

    strbuilder_push_owned(&strBld, tailChunk);

    ResultOwnedStr strRes = strbuilder_get_string(&strBld);

    if (strRes.error)
        return (ResultOwnedStr){
            .value = NULL,
            .error = strRes.error,
        };

    HeapStr result = strRes.value;

    strbuilder_deinit(&strBld);

    return (ResultOwnedStr){
        .value = result,
        .error = ERR_OK,
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
ibool string_starts_with(ConstStr s, ConstStr what)
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
ibool string_ends_with(ConstStr s, ConstStr what)
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

ibool char_is_alpha(const i8 c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

ibool char_is_digit(const i8 c)
{
    return (c >= '0' && c <= '9');
}

ibool char_is_alphanum(const i8 c)
{
    return (char_is_alpha(c) || char_is_digit(c));
}

ibool char_is_whitespace(const i8 c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

/**
 * @brief Trims whitespace characters from the start and/or end of a string.
 * Creates a new string with the trimmed result.
 *
 * Memory is owned by the caller and should be freed using the same allocator.
 *
 * ```c
 * ConstStr myStr = "  Hello, World!  ";
 * ResultOwnedStr trimmed = string_trim_whitespace(&c_allocator, myStr, true, true);
 * if (trimmed.error) // Error!
 * // trimmed.value == "Hello, World!"
 *
 * ConstStr myStr2 = "  Hello, World!  ";
 * ResultOwnedStr trimStart = string_trim_whitespace(&c_allocator, myStr2, true, false);
 * if (trimStart.error) // Error!
 * // trimStart.value == "Hello, World!  "
 * ```
 *
 * @param alloc Allocator to use for the new string
 * @param s String to trim
 * @param start If !0 (true), trim from the start of the string
 * @param end If !0 (true), trim from the end of the string
 * @return ResultOwnedStr
 */
ResultOwnedStr string_trim_whitespace(Allocator *alloc, ConstStr s, ibool start, ibool end)
{
    if (!alloc || !s)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_INVALID_PARAMETER,
        };

    u64 len = string_size(s);
    u64 startIdx = 0;
    u64 endIdx = len;

    if (start)
    {
        while (startIdx < len && char_is_whitespace(s[startIdx]))
        {
            ++startIdx;
        }
    }

    if (end)
    {
        while (endIdx > startIdx && char_is_whitespace(s[endIdx - 1]))
        {
            --endIdx;
        }
    }

    if (startIdx >= endIdx)
    {
        HeapStr newStr = string_dupe_noresult(alloc, "");

        if (!newStr)
            return (ResultOwnedStr){
                .value = NULL,
                .error = ERR_OUT_OF_MEMORY,
            };

        return (ResultOwnedStr){
            .value = newStr,
            .error = ERR_OK,
        };
    }

    u64 newLen = endIdx - startIdx;
    HeapStr newStr = alloc->alloc(alloc, newLen + 1);

    if (!newStr)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_OUT_OF_MEMORY,
        };

    string_copy_n_unsafe(s + startIdx, newStr, newLen, false);
    newStr[newLen] = (i8)0;

    return (ResultOwnedStr){
        .value = newStr,
        .error = ERR_OK,
    };
}

i8 digit_to_char(const i16 i)
{
    if (i > 9)
        return 0;

    return "0123456789"[i];
}

ResultOwnedStr string_from_int(Allocator *alloc, const i64 i)
{
    char buf[20];
    i64 n = i;
    i16 idx = 0;

    if (n == 0)
    {
        HeapStr intStr = alloc->alloc(alloc, 2);
        intStr[0] = '0';
        intStr[1] = '\0';
        return (ResultOwnedStr){
            .value = intStr,
            .error = ERR_OK,
        };
    }

    if (n < 0)
    {
        buf[0] = '-';
        ++idx;
        n = -n;
    }

    while (n != 0)
    {
        i16 d = n % 10;
        buf[idx++] = digit_to_char(d);
        n /= 10;
    }

    buf[idx] = '\0';

    HeapStr intStr = alloc->alloc(alloc, idx + 1);
    string_copy_unsafe(buf, intStr);

    return (ResultOwnedStr){
        .value = intStr,
        .error = ERR_OK,
    };
}

ResultOwnedStr string_from_uint(Allocator *alloc, const u64 i)
{
    char buf[20];
    u64 n = i;
    i16 idx = 0;

    if (n == 0)
    {
        HeapStr intStr = alloc->alloc(alloc, 2);
        intStr[0] = '0';
        intStr[1] = '\0';
        return (ResultOwnedStr){
            .value = intStr,
            .error = ERR_OK,
        };
    }

    while (n != 0)
    {
        i16 d = n % 10;
        buf[idx++] = digit_to_char(d);
        n /= 10;
    }

    buf[idx] = '\0';

    HeapStr intStr = alloc->alloc(alloc, idx + 1);
    string_copy_unsafe(buf, intStr);

    return (ResultOwnedStr){
        .value = intStr,
        .error = ERR_OK,
    };
}

ResultOwnedStr string_from_float(Allocator *alloc, const f64 flt, const u64 precision)
{
    f64 d = flt;

    i64 intPart = (i64)d;
    f64 fracPart = d - (f64)intPart;

    ResultOwnedStr strIntPart = string_from_int(alloc, intPart);

    if (precision <= 0 || strIntPart.error != ERR_OK)
    {
        return strIntPart;
    }

    f64 scale = 1.0;
    for (u64 i = 0; i < precision; ++i)
        scale *= 10.0;

    fracPart *= scale;
    u64 fracInt = (u64)(fracPart + 0.5); // rounding

    // Ensure leading zeros in fractional part
    i64 div = 1;
    for (int i = 1; i < precision; ++i)
        div *= 10;

    u64 zeroes = 0;
    while (div > fracInt && div > 1)
    {
        ++zeroes;
        div /= 10;
    }

    ResultOwnedStr zeroesStr = string_alloc(alloc, zeroes, '0');

    if (zeroesStr.error != ERR_OK)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_OUT_OF_MEMORY,
        };

    ResultOwnedStr strDecPart = string_from_uint(alloc, fracInt);

    if (strDecPart.error != ERR_OK)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_OUT_OF_MEMORY,
        };

    u64 intPartSize = string_size(strIntPart.value);
    u64 decPartSize = string_size(strDecPart.value);

    u64 totalSize = intPartSize + decPartSize + 2;
    HeapStr finalStr = alloc->alloc(alloc, totalSize);

    if (finalStr == NULL)
        return (ResultOwnedStr){
            .value = NULL,
            .error = ERR_OUT_OF_MEMORY,
        };

    string_copy_n_unsafe(strIntPart.value, finalStr, intPartSize, false);
    finalStr[intPartSize] = '.';
    string_copy_n_unsafe(strDecPart.value, finalStr + intPartSize + 1, decPartSize, true);

    return (ResultOwnedStr){
        .value = finalStr,
        .error = ERR_OK,
    };
}

ResultI64 string_parse_int(ConstStr s)
{
    if (!s || *s == '\0')
        return (ResultI64){
            .value = 0,
            .error = ERR_INVALID_PARAMETER,
        };

    const i8 *ptr = s;
    i64 result = 0;
    i64 sign = 1;

    while (char_is_whitespace(*ptr))
        ++ptr;

    if (*ptr == '-' || *ptr == '+')
    {
        if (*ptr == '-')
            sign = -1;
        ++ptr;
    }

    if (!char_is_digit(*ptr))
        return (ResultI64){
            .value = 0,
            .error = ERR_UNEXPECTED_BYTE,
        };

    while (char_is_digit(*ptr))
    {
        i64 newVal = result * 10 + (*ptr - '0');

        if (newVal < result)
            return (ResultI64){
                .value = 0,
                .error = ERR_WOULD_OVERFLOW,
            };

        result = newVal;
        ++ptr;
    }

    if (*ptr != '\0' && !char_is_whitespace(*ptr))
        return (ResultI64){
            .value = 0,
            .error = ERR_UNEXPECTED_BYTE,
        };

    return (ResultI64){
        .value = result * sign,
        .error = ERR_OK,
    };
}

ResultU64 string_parse_uint(ConstStr s)
{
    if (!s || *s == '\0')
        return (ResultU64){
            .value = 0,
            .error = ERR_INVALID_PARAMETER,
        };

    const i8 *ptr = s;
    u64 result = 0;

    while (char_is_whitespace(*ptr))
        ++ptr;

    if (!char_is_digit(*ptr))
        return (ResultU64){
            .value = 0,
            .error = ERR_UNEXPECTED_BYTE,
        };

    while (char_is_digit(*ptr))
    {
        u64 newVal = result * 10 + (*ptr - '0');

        if (newVal < result)
            return (ResultU64){
                .value = 0,
                .error = ERR_WOULD_OVERFLOW,
            };

        result = newVal;
        ++ptr;
    }

    if (*ptr != '\0' && !char_is_whitespace(*ptr))
        return (ResultU64){
            .value = 0,
            .error = ERR_UNEXPECTED_BYTE,
        };

    return (ResultU64){
        .value = result,
        .error = ERR_OK,
    };
}

ResultF64 string_parse_float(ConstStr s)
{
    if (!s || *s == '\0')
        return (ResultF64){
            .value = 0.0,
            .error = ERR_INVALID_PARAMETER,
        };

    const i8 *ptr = s;
    f64 result = 0.0;
    f64 sign = 1.0;

    while (char_is_whitespace(*ptr))
        ++ptr;

    if (*ptr == '-' || *ptr == '+')
    {
        if (*ptr == '-')
            sign = -1.0;
        ++ptr;
    }

    if (!char_is_digit(*ptr) && *ptr != '.')
        return (ResultF64){
            .value = 0.0,
            .error = ERR_UNEXPECTED_BYTE,
        };

    while (char_is_digit(*ptr))
    {
        f64 newVal = result * 10.0 + (*ptr - '0');

        if (newVal < result)
            return (ResultF64){
                .value = 0.0,
                .error = ERR_WOULD_OVERFLOW,
            };

        result = newVal;
        ++ptr;
    }

    if (*ptr == '.')
    {
        ++ptr;
        f64 fraction = 0.0;
        f64 divisor = 10.0;

        if (!char_is_digit(*ptr))
            return (ResultF64){
                .value = 0.0,
                .error = ERR_UNEXPECTED_BYTE,
            };

        while (char_is_digit(*ptr))
        {
            f64 newFrac = fraction + (*ptr - '0') / divisor;

            if (newFrac < fraction)
                return (ResultF64){
                    .value = 0.0,
                    .error = ERR_WOULD_OVERFLOW,
                };

            fraction = newFrac;
            divisor *= 10.0;
            ++ptr;
        }
        result += fraction;
    }

    if (*ptr != '\0' && !char_is_whitespace(*ptr))
        return (ResultF64){
            .value = 0.0,
            .error = ERR_UNEXPECTED_BYTE,
        };

    return (ResultF64){
        .value = result * sign,
        .error = ERR_OK,
    };
}
