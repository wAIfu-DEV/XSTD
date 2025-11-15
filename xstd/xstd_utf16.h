#pragma once

#include "xstd_core.h"
#include "xstd_error.h"
#include "xstd_alloc.h"
#include "xstd_utf8.h"

typedef struct _utf16_iter
{
    Utf16ConstStr ptr; // Current code unit within the range (may point at end sentinel)
    Utf16ConstStr end; // One past the last code unit in the range, or NULL for nul-terminated sources
} Utf16Iter;

typedef struct
{
    Utf16Iter value;
    Error error;
} ResultUtf16Iter;

typedef struct _utf16_codepoint
{
    u32 codepoint; // Unicode scalar value
    u8 width;      // Number of UTF-16 code units consumed from the original string
} Utf16Codepoint;

typedef struct _result_utf16_codepoint
{
    Utf16Codepoint value;
    Error error;
} ResultUtf16Codepoint;

typedef struct _result_owned_utf16_str
{
    Utf16OwnedStr value;
    Error error;
} ResultUtf16OwnedStr;

static inline ResultUtf16Iter utf16_iter_buff(Utf16Buff buff)
{
    if (!buff.units)
        return (ResultUtf16Iter){
            .error = X_ERR_EXT("utf16", "utf16_iter_buff",
                ERR_INVALID_PARAMETER, "null buffer"),
        };

    return (ResultUtf16Iter){
        .value = (Utf16Iter){
            .ptr = buff.units,
            .end = buff.units + buff.size,
        },
        .error = X_ERR_OK,
    };
}

static inline ResultUtf16Iter utf16_iter_str(Utf16ConstStr s)
{
    if (!s)
        return (ResultUtf16Iter){
            .error = X_ERR_EXT("utf16", "utf16_iter_buff",
                ERR_INVALID_PARAMETER, "null string"),
        };

    return (ResultUtf16Iter){
        .value = (Utf16Iter){
            .ptr = s,
            .end = NULL,
        },
        .error = X_ERR_OK,
    };
}

static inline ibool utf16_iter_has_next(const Utf16Iter *it)
{
    if (!it || !it->ptr)
        return false;

    if (it->end)
        return it->ptr < it->end;

    return *it->ptr != 0;
}

static inline ResultUtf16Codepoint _utf16_decode(Utf16ConstStr ptr, Utf16ConstStr end)
{
    if (!ptr)
        return (ResultUtf16Codepoint){
            .value = {
                .codepoint = 0,
                .width = 0,
            },
            .error = X_ERR_EXT("utf16", "_utf16_decode",
                ERR_INVALID_PARAMETER, "null iterator pointer"),
        };

    if (end)
    {
        if (ptr >= end)
            return (ResultUtf16Codepoint){
                .value = {
                    .codepoint = 0,
                    .width = 0,
                },
                .error = X_ERR_EXT("utf16", "_utf16_decode",
                    ERR_RANGE_ERROR, "iterator at end"),
            };
    }
    else if (*ptr == 0)
    {
        return (ResultUtf16Codepoint){
            .value = {
                .codepoint = 0,
                .width = 0,
            },
            .error = X_ERR_EXT("utf16", "_utf16_decode",
                ERR_RANGE_ERROR, "iterator at end"),
        };
    }

    const u16 first = (u16)*ptr;

    if (first >= 0xD800u && first <= 0xDBFFu)
    {
        const Utf16ConstStr next = ptr + 1;

        if (end)
        {
            if (next >= end)
                return (ResultUtf16Codepoint){
                    .value = {
                        .codepoint = 0,
                        .width = 0,
                    },
                    .error = X_ERR_EXT("utf16", "_utf16_decode",
                        ERR_RANGE_ERROR, "unterminated utf16 surrogate"),
                };
        }
        else if (*next == 0)
        {
            return (ResultUtf16Codepoint){
                .value = {
                    .codepoint = 0,
                    .width = 0,
                },
                .error = X_ERR_EXT("utf16", "_utf16_decode",
                    ERR_RANGE_ERROR, "unterminated utf16 surrogate"),
            };
        }

        const u16 second = (u16)*next;

        if (second < 0xDC00u || second > 0xDFFFu)
            return (ResultUtf16Codepoint){
                .value = {
                    .codepoint = 0,
                    .width = 0,
                },
                .error = X_ERR_EXT("utf16", "_utf16_decode",
                    ERR_UNEXPECTED_BYTE, "invalid utf16 low surrogate"),
            };

        const u32 codepoint =
            0x10000u + (((u32)first - 0xD800u) << 10) + ((u32)second - 0xDC00u);

        return (ResultUtf16Codepoint){
            .value = {
                .codepoint = codepoint,
                .width = 2,
            },
            .error = X_ERR_OK,
        };
    }

    if (first >= 0xDC00u && first <= 0xDFFFu)
        return (ResultUtf16Codepoint){
            .value = {
                .codepoint = 0,
                .width = 0,
            },
            .error = X_ERR_EXT("utf16", "_utf16_decode",
                ERR_PARSE_ERROR, "unexpected utf16 low surrogate"),
        };

    return (ResultUtf16Codepoint){
        .value = {
            .codepoint = (u32)first,
            .width = 1,
        },
        .error = X_ERR_OK,
    };
}

static inline ResultUtf16Codepoint utf16_iter_peek(const Utf16Iter *it)
{
    if (!it || !it->ptr)
        return (ResultUtf16Codepoint){
            .value = {
                .codepoint = 0,
                .width = 0,
            },
            .error = X_ERR_EXT("utf16", "utf16_iter_peek",
                ERR_INVALID_PARAMETER, "null iterator"),
        };

    return _utf16_decode(it->ptr, it->end);
}

static inline ResultUtf16Codepoint utf16_iter_next(Utf16Iter *it)
{
    if (!it || !it->ptr)
        return (ResultUtf16Codepoint){
            .value = {
                .codepoint = 0,
                .width = 0,
            },
            .error = X_ERR_EXT("utf16", "utf16_iter_next",
                ERR_INVALID_PARAMETER, "null iterator"),
        };

    ResultUtf16Codepoint res = _utf16_decode(it->ptr, it->end);

    if (res.error.code == ERR_OK)
        it->ptr += res.value.width;

    return res;
}

static inline void utf16_iter_advance_bytes(Utf16Iter *it, u64 n)
{
    if (!it || !it->ptr)
        return;

    if (it->end)
    {
        if (n > (u64)(it->end - it->ptr))
        {
            it->ptr = it->end;
            return;
        }
        it->ptr += n;
        return;
    }

    while (n && *it->ptr)
    {
        ++it->ptr;
        --n;
    }
}

static inline ResultUtf16OwnedStr utf8_buff_to_utf16(Allocator *a, ConstBuff buff)
{
    if (!a)
        return (ResultUtf16OwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("utf16", "utf8_buff_to_utf16",
                ERR_INVALID_PARAMETER, "null allocator"),
        };

    if (buff.size == 0)
    {
        Utf16OwnedStr out = (Utf16OwnedStr)a->alloc(a, sizeof(i16));
        if (!out)
            return (ResultUtf16OwnedStr){
                .value = NULL,
                .error = X_ERR_EXT("utf16", "utf8_buff_to_utf16",
                    ERR_OUT_OF_MEMORY, "alloc failure"),
            };
        out[0] = 0;
        return (ResultUtf16OwnedStr){
            .value = out,
            .error = X_ERR_OK,
        };
    }

    ResultUtf8Iter iterRes = utf8_iter_buff(buff);
    if (iterRes.error.code != ERR_OK)
        return (ResultUtf16OwnedStr){
            .value = NULL,
            .error = iterRes.error,
        };

    Utf8Iter countIt = iterRes.value;
    u64 unitsNeeded = 0;

    while (utf8_iter_has_next(&countIt))
    {
        ResultUtf8Codepoint cpRes = utf8_iter_next(&countIt);
        if (cpRes.error.code != ERR_OK)
            return (ResultUtf16OwnedStr){
                .value = NULL,
                .error = cpRes.error,
            };

        u64 add = (cpRes.value.codepoint >= 0x10000u) ? 2u : 1u;

        if (unitsNeeded > EnumMaxVal.U64 - add - 1u)
            return (ResultUtf16OwnedStr){
                .value = NULL,
                .error = X_ERR_EXT("utf16", "utf8_buff_to_utf16",
                    ERR_WOULD_OVERFLOW, "utf16 size overflow"),
            };

        unitsNeeded += add;
    }

    u64 totalUnits = unitsNeeded + 1u;

    if (totalUnits > EnumMaxVal.U64 / (u64)sizeof(i16))
        return (ResultUtf16OwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("utf16", "utf8_buff_to_utf16",
                ERR_WOULD_OVERFLOW, "utf16 alloc overflow"),
        };

    Utf16OwnedStr out = (Utf16OwnedStr)a->alloc(a, totalUnits * sizeof(wChar));
    if (!out)
        return (ResultUtf16OwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("utf16", "utf8_buff_to_utf16",
                ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    Utf8Iter readIt = iterRes.value;
    wChar *write = out;

    while (utf8_iter_has_next(&readIt))
    {
        ResultUtf8Codepoint cpRes = utf8_iter_next(&readIt);
        if (cpRes.error.code != ERR_OK)
        {
            a->free(a, out);
            return (ResultUtf16OwnedStr){
                .value = NULL,
                .error = cpRes.error,
            };
        }

        u32 codepoint = cpRes.value.codepoint;

        if (codepoint < 0x10000u)
        {
            *write++ = (wChar)codepoint;
        }
        else
        {
            codepoint -= 0x10000u;
            *write++ = (wChar)((codepoint >> 10) + 0xD800u);
            *write++ = (wChar)((codepoint & 0x3FFu) + 0xDC00u);
        }
    }

    *write = 0;

    return (ResultUtf16OwnedStr){
        .value = out,
        .error = X_ERR_OK,
    };
}

static inline ResultUtf16OwnedStr utf8_to_utf16(Allocator *a, ConstStr s)
{
    if (!a || !s)
        return (ResultUtf16OwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("utf16", "utf8_to_utf16",
                ERR_INVALID_PARAMETER, "null argument"),
        };

    u64 len = 0;
    while (s[len])
        ++len;

    return utf8_buff_to_utf16(a, (ConstBuff){
        .bytes = (i8*)s,
        .size = len,
    });
}
