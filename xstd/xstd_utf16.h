#pragma once

#include "xstd_core.h"
#include "xstd_result.h"
#include "xstd_error.h"
#include "xstd_alloc.h"
#include "xstd_utf8.h"

typedef struct _utf16_iter
{
    Utf16ConstStr ptr; // Current code unit within the range (may point at end sentinel)
    Utf16ConstStr end; // One past the last code unit in the range, or NULL for nul-terminated sources
} Utf16Iter;

result_define(Utf16Iter, Utf16Iter);

typedef struct _utf16_codepoint
{
    u32 codepoint; // Unicode scalar value
    u8 width;      // Number of UTF-16 code units consumed from the original string
} Utf16Codepoint;

result_define(Utf16Codepoint, Utf16Cp);
result_define(Utf16OwnedStr, Utf16OwnedStr);

static inline result_type(Utf16Iter) utf16_iter_buff(Utf16Buff buff)
{
    if (!buff.units)
        return result_err(Utf16Iter, X_ERR_EXT("utf16", "utf16_iter_buff", ERR_INVALID_PARAMETER, "null buffer"));

    Utf16Iter it = {
        .ptr = buff.units,
        .end = buff.units + buff.size,
    };
    return result_ok(Utf16Iter, it);
}

static inline result_type(Utf16Iter) utf16_iter_str(Utf16ConstStr s)
{
    if (!s)
        return result_err(Utf16Iter, X_ERR_EXT("utf16", "utf16_iter_str", ERR_INVALID_PARAMETER, "null string"));

    Utf16Iter it = {
        .ptr = s,
        .end = NULL,
    };
    return result_ok(Utf16Iter, it);
}

static inline Bool utf16_iter_has_next(const Utf16Iter *it)
{
    if (!it || !it->ptr)
        return false;

    if (it->end)
        return it->ptr < it->end;

    return *it->ptr != 0;
}

static inline result_type(Utf16Cp) _utf16_decode(Utf16ConstStr ptr, Utf16ConstStr end)
{
    if (!ptr)
        return result_err(Utf16Cp, X_ERR_EXT("utf16", "_utf16_decode", ERR_INVALID_PARAMETER, "null iterator pointer"));

    if (end)
    {
        if (ptr >= end)
            return result_err(Utf16Cp, X_ERR_EXT("utf16", "_utf16_decode", ERR_RANGE_ERROR, "iterator at end"));
    }
    else if (*ptr == 0)
    {
        return result_err(Utf16Cp, X_ERR_EXT("utf16", "_utf16_decode", ERR_RANGE_ERROR, "iterator at end"));
    }

    const u16 first = (u16)*ptr;

    if (first >= 0xD800u && first <= 0xDBFFu)
    {
        const Utf16ConstStr next = ptr + 1;

        if (end)
        {
            if (next >= end)
                return result_err(Utf16Cp, X_ERR_EXT("utf16", "_utf16_decode", ERR_RANGE_ERROR, "unterminated utf16 surrogate"));
        }
        else if (*next == 0)
        {
            return result_err(Utf16Cp, X_ERR_EXT("utf16", "_utf16_decode", ERR_RANGE_ERROR, "unterminated utf16 surrogate"));
        }

        const u16 second = (u16)*next;

        if (second < 0xDC00u || second > 0xDFFFu)
            return result_err(Utf16Cp, X_ERR_EXT("utf16", "_utf16_decode", ERR_UNEXPECTED_BYTE, "invalid utf16 low surrogate"));

        const u32 codepoint =
            0x10000u + (((u32)first - 0xD800u) << 10) + ((u32)second - 0xDC00u);

        Utf16Codepoint cp = {
            .codepoint = codepoint,
            .width = 2,
        };
        return result_ok(Utf16Cp, cp);
    }

    if (first >= 0xDC00u && first <= 0xDFFFu)
        return result_err(Utf16Cp, X_ERR_EXT("utf16", "_utf16_decode", ERR_PARSE_ERROR, "unexpected utf16 low surrogate"));

    Utf16Codepoint cp = {
        .codepoint = (u32)first,
        .width = 1,
    };
    return result_ok(Utf16Cp, cp);
}

static inline result_type(Utf16Cp) utf16_iter_peek(const Utf16Iter *it)
{
    if (!it || !it->ptr)
        return result_err(Utf16Cp, X_ERR_EXT("utf16", "utf16_iter_peek", ERR_INVALID_PARAMETER, "null iterator"));

    return _utf16_decode(it->ptr, it->end);
}

static inline result_type(Utf16Cp) utf16_iter_next(Utf16Iter *it)
{
    if (!it || !it->ptr)
        return result_err(Utf16Cp, X_ERR_EXT("utf16", "utf16_iter_next", ERR_INVALID_PARAMETER, "null iterator"));

    result_type(Utf16Cp) res = _utf16_decode(it->ptr, it->end);

    if (!res.isErr)
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

static inline result_type(Utf16OwnedStr) utf8_buff_to_utf16(Allocator *a, ConstBuff buff)
{
    if (!a)
        return result_err(Utf16OwnedStr, X_ERR_EXT("utf16", "utf8_buff_to_utf16", ERR_INVALID_PARAMETER, "null allocator"));

    if (buff.size == 0)
    {
        Utf16OwnedStr out = (Utf16OwnedStr)a->alloc(a, sizeof(i16));
        if (!out)
            return result_err(Utf16OwnedStr, X_ERR_EXT("utf16", "utf8_buff_to_utf16", ERR_OUT_OF_MEMORY, "alloc failure"));

        out[0] = 0;
        return result_ok(Utf16OwnedStr, out);
    }

    result_type(Utf8Iter) iterRes = utf8_iter_buff(buff);
    if (iterRes.isErr)
        return result_err(Utf16OwnedStr, iterRes.err);

    Utf8Iter countIt = iterRes.value;
    u64 unitsNeeded = 0;

    while (utf8_iter_has_next(&countIt))
    {
        result_type(Utf8Cp) cpRes = utf8_iter_next(&countIt);
        if (cpRes.isErr)
            return result_err(Utf16OwnedStr, cpRes.err);

        u64 add = (cpRes.value.codepoint >= 0x10000u) ? 2u : 1u;

        if (unitsNeeded > EnumMaxVal.U64 - add - 1u)
            return result_err(Utf16OwnedStr, X_ERR_EXT("utf16", "utf8_buff_to_utf16",
                    ERR_WOULD_OVERFLOW, "utf16 size overflow"));

        unitsNeeded += add;
    }

    u64 totalUnits = unitsNeeded + 1u;

    if (totalUnits > EnumMaxVal.U64 / (u64)sizeof(i16))
        return result_err(Utf16OwnedStr, X_ERR_EXT("utf16", "utf8_buff_to_utf16",
                ERR_WOULD_OVERFLOW, "utf16 alloc overflow"));

    Utf16OwnedStr out = (Utf16OwnedStr)a->alloc(a, totalUnits * sizeof(wChar));
    if (!out)
        return result_err(Utf16OwnedStr, X_ERR_EXT("utf16", "utf8_buff_to_utf16",
                ERR_OUT_OF_MEMORY, "alloc failure"));

    Utf8Iter readIt = iterRes.value;
    wChar *write = out;

    while (utf8_iter_has_next(&readIt))
    {
        result_type(Utf8Cp) cpRes = utf8_iter_next(&readIt);
        if (cpRes.isErr)
        {
            a->free(a, out);
            return result_err(Utf16OwnedStr, cpRes.err);
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
    return result_ok(Utf16OwnedStr, out);
}

static inline result_type(Utf16OwnedStr) utf8_to_utf16(Allocator *a, ConstStr s)
{
    if (!a || !s)
        return result_err(Utf16OwnedStr, X_ERR_EXT("utf16", "utf8_to_utf16",
                ERR_INVALID_PARAMETER, "null argument"));

    u64 len = 0;
    while (s[len])
        ++len;

    return utf8_buff_to_utf16(a, (ConstBuff){
        .bytes = (i8*)s,
        .size = len,
    });
}
