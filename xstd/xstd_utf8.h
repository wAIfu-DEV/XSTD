#pragma once

#include "xstd_core.h"
#include "xstd_error.h"
#include "xstd_alloc.h"
#include "xstd_result.h"

typedef struct _utf8_iter
{
    ConstStr ptr; // Current byte within the range (may point at end sentinel)
    ConstStr end; // One past the last byte in the range, or NULL for nul-terminated sources
} Utf8Iter;

typedef struct
{
    Utf8Iter value;
    Error error;
} ResultUtf8Iter;

typedef struct _utf8_codepoint
{
    u32 codepoint; // Unicode scalar value
    u8 width;      // Number of bytes consumed from the original string
} Utf8Codepoint;

typedef struct _result_utf8_codepoint
{
    Utf8Codepoint value;
    Error error;
} ResultUtf8Codepoint;

static inline ResultUtf8Iter utf8_iter_buff(ConstBuff buff)
{
    if (!buff.bytes)
        return (ResultUtf8Iter){
            .error = X_ERR_EXT("utf8", "utf8_iter_buff",
                ERR_INVALID_PARAMETER, "null buffer"),
        };

    return (ResultUtf8Iter){
        .value = (Utf8Iter){
            .ptr = buff.bytes,
            .end = buff.bytes + buff.size,
        },
        .error = X_ERR_OK,
    };
}

static inline ResultUtf8Iter utf8_iter_str(ConstStr s)
{
    if (!s)
        return (ResultUtf8Iter){
            .error = X_ERR_EXT("utf8", "utf8_iter_buff",
                ERR_INVALID_PARAMETER, "null string"),
        };

    return (ResultUtf8Iter){
        .value = (Utf8Iter){
            .ptr = s,
            .end = NULL,
        },
        .error = X_ERR_OK,
    };
}

static inline ibool utf8_iter_has_next(const Utf8Iter *it)
{
    if (!it || !it->ptr)
        return false;

    if (it->end)
        return it->ptr < it->end;

    return *it->ptr != '\0';
}

static inline ResultUtf8Codepoint _utf8_decode(ConstStr ptr, ConstStr end)
{
    if (!ptr)
        return (ResultUtf8Codepoint){
            .value = {
                .codepoint = 0,
                .width = 0,
            },
            .error = X_ERR_EXT("utf8", "_utf8_decode",
                ERR_INVALID_PARAMETER, "null iterator pointer"),
        };

    if (end)
    {
        if (ptr >= end)
            return (ResultUtf8Codepoint){
                .value = {
                    .codepoint = 0,
                    .width = 0,
                },
                .error = X_ERR_EXT("utf8", "_utf8_decode",
                    ERR_RANGE_ERROR, "iterator at end"),
            };
    }
    else if (*ptr == '\0')
    {
        return (ResultUtf8Codepoint){
            .value = {
                .codepoint = 0,
                .width = 0,
            },
            .error = X_ERR_EXT("utf8", "_utf8_decode",
                ERR_RANGE_ERROR, "iterator at end"),
        };
    }

    const u8 *bytes = (const u8 *)ptr;
    const u8 first = bytes[0];

    u8 width = 0;
    u32 codepoint = 0;

    if ((first & 0x80u) == 0x00u)
    {
        width = 1;
        codepoint = first;
    }
    else if ((first & 0xE0u) == 0xC0u)
    {
        width = 2;
        codepoint = (u32)(first & 0x1Fu);
    }
    else if ((first & 0xF0u) == 0xE0u)
    {
        width = 3;
        codepoint = (u32)(first & 0x0Fu);
    }
    else if ((first & 0xF8u) == 0xF0u)
    {
        width = 4;
        codepoint = (u32)(first & 0x07u);
    }
    else
    {
        return (ResultUtf8Codepoint){
            .value = {
                .codepoint = 0,
                .width = 0,
            },
            .error = X_ERR_EXT("utf8", "_utf8_decode",
                ERR_UNEXPECTED_BYTE, "invalid utf8 lead byte"),
        };
    }

    if (width > 1)
    {
        if (end)
        {
            if ((u64)(end - ptr) < width)
                return (ResultUtf8Codepoint){
                    .value = {
                        .codepoint = 0,
                        .width = 0,
                    },
                    .error = X_ERR_EXT("utf8", "_utf8_decode",
                        ERR_RANGE_ERROR, "unterminated utf8 sequence"),
                };
        }
        else
        {
            for (u8 idx = 1; idx < width; ++idx)
            {
                if ((u8)ptr[idx] == 0u)
                    return (ResultUtf8Codepoint){
                        .value = {
                            .codepoint = 0,
                            .width = 0,
                        },
                        .error = X_ERR_EXT("utf8", "_utf8_decode",
                            ERR_RANGE_ERROR, "unterminated utf8 sequence"),
                    };
            }
        }
    }

    for (u8 i = 1; i < width; ++i)
    {
        const u8 cont = (u8)bytes[i];
        if ((cont & 0xC0u) != 0x80u)
        {
            return (ResultUtf8Codepoint){
                .value = {
                    .codepoint = 0,
                    .width = 0,
                },
                .error = X_ERR_EXT("utf8", "_utf8_decode",
                    ERR_UNEXPECTED_BYTE, "invalid utf8 continuation"),
            };
        }

        codepoint = (codepoint << 6) | (u32)(cont & 0x3Fu);
    }

    switch (width)
    {
    case 1:
        break;
    case 2:
        if (codepoint < 0x80u)
            return (ResultUtf8Codepoint){
                .value = {
                    .codepoint = 0,
                    .width = 0,
                },
                .error = X_ERR_EXT("utf8", "_utf8_decode",
                    ERR_UNEXPECTED_BYTE, "overlong utf8 sequence"),
            };
        break;
    case 3:
        if (codepoint < 0x800u)
            return (ResultUtf8Codepoint){
                .value = {
                    .codepoint = 0,
                    .width = 0,
                },
                .error = X_ERR_EXT("utf8", "_utf8_decode",
                    ERR_UNEXPECTED_BYTE, "overlong utf8 sequence"),
            };
        if (codepoint >= 0xD800u && codepoint <= 0xDFFFu)
            return (ResultUtf8Codepoint){
                .value = {
                    .codepoint = 0,
                    .width = 0,
                },
                .error = X_ERR_EXT("utf8", "_utf8_decode",
                    ERR_RANGE_ERROR, "utf16 surrogate codepoint"),
            };
        break;
    case 4:
        if (codepoint < 0x10000u)
            return (ResultUtf8Codepoint){
                .value = {
                    .codepoint = 0,
                    .width = 0,
                },
                .error = X_ERR_EXT("utf8", "_utf8_decode",
                    ERR_UNEXPECTED_BYTE, "overlong utf8 sequence"),
            };
        if (codepoint > 0x10FFFFu)
            return (ResultUtf8Codepoint){
                .value = {
                    .codepoint = 0,
                    .width = 0,
                },
                .error = X_ERR_EXT("utf8", "_utf8_decode",
                    ERR_RANGE_ERROR, "codepoint out of range"),
            };
        break;
    default:
        return (ResultUtf8Codepoint){
            .value = {
                .codepoint = 0,
                .width = 0,
            },
            .error = X_ERR_EXT("utf8", "_utf8_decode",
                ERR_PARSE_ERROR, "unsupported utf8 width"),
        };
    }

    return (ResultUtf8Codepoint){
        .value = {
            .codepoint = codepoint,
            .width = width,
        },
        .error = X_ERR_OK,
    };
}

static inline ResultUtf8Codepoint utf8_iter_peek(const Utf8Iter *it)
{
    if (!it || !it->ptr)
        return (ResultUtf8Codepoint){
            .value = {
                .codepoint = 0,
                .width = 0,
            },
            .error = X_ERR_EXT("utf8", "utf8_iter_peek",
                ERR_INVALID_PARAMETER, "null iterator"),
        };

    return _utf8_decode(it->ptr, it->end);
}

static inline ResultUtf8Codepoint utf8_iter_next(Utf8Iter *it)
{
    if (!it || !it->ptr)
        return (ResultUtf8Codepoint){
            .value = {
                .codepoint = 0,
                .width = 0,
            },
            .error = X_ERR_EXT("utf8", "utf8_iter_next",
                ERR_INVALID_PARAMETER, "null iterator"),
        };

    ResultUtf8Codepoint res = _utf8_decode(it->ptr, it->end);

    if (res.error.code == ERR_OK)
        it->ptr += res.value.width;

    return res;
}

static inline void utf8_iter_advance_bytes(Utf8Iter *it, u64 n)
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

static inline ResultOwnedStr utf16_buff_to_utf8(Allocator *a, Utf16Buff buff)
{
    if (!a)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("utf8", "utf16_buff_to_utf8",
                ERR_INVALID_PARAMETER, "null allocator"),
        };

    if (buff.size == 0)
    {
        HeapStr out = (HeapStr)a->alloc(a, 1);
        if (!out)
            return (ResultOwnedStr){
                .value = NULL,
                .error = X_ERR_EXT("utf8", "utf16_buff_to_utf8",
                    ERR_OUT_OF_MEMORY, "alloc failure"),
            };
        out[0] = 0;
        return (ResultOwnedStr){
            .value = out,
            .error = X_ERR_OK,
        };
    }

    if (!buff.units)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("utf8", "utf16_buff_to_utf8",
                ERR_INVALID_PARAMETER, "null buffer"),
        };

    Utf16ConstStr ptr = (Utf16ConstStr)buff.units;
    Utf16ConstStr end = ((Utf16ConstStr)buff.units) + buff.size;
    u64 bytesNeeded = 0;

    while (ptr < end)
    {
        u16 first = (u16)*ptr++;
        u64 add = 0;

        if (first >= 0xD800u && first <= 0xDBFFu)
        {
            if (ptr >= end)
                return (ResultOwnedStr){
                    .value = NULL,
                    .error = X_ERR_EXT("utf8", "utf16_buff_to_utf8",
                        ERR_RANGE_ERROR, "unterminated utf16 surrogate"),
                };

            u16 second = (u16)*ptr;
            if (second < 0xDC00u || second > 0xDFFFu)
                return (ResultOwnedStr){
                    .value = NULL,
                    .error = X_ERR_EXT("utf8", "utf16_buff_to_utf8",
                        ERR_UNEXPECTED_BYTE, "invalid utf16 low surrogate"),
                };

            ++ptr;
            add = 4;
        }
        else if (first >= 0xDC00u && first <= 0xDFFFu)
        {
            return (ResultOwnedStr){
                .value = NULL,
                .error = X_ERR_EXT("utf8", "utf16_buff_to_utf8",
                    ERR_PARSE_ERROR, "unexpected utf16 low surrogate"),
            };
        }
        else if (first <= 0x7Fu)
        {
            add = 1;
        }
        else if (first <= 0x7FFu)
        {
            add = 2;
        }
        else
        {
            add = 3;
        }

        if (bytesNeeded > MaxVals.U64 - add)
            return (ResultOwnedStr){
                .value = NULL,
                .error = X_ERR_EXT("utf8", "utf16_buff_to_utf8",
                    ERR_WOULD_OVERFLOW, "utf8 size overflow"),
            };

        bytesNeeded += add;
    }

    if (bytesNeeded == MaxVals.U64)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("utf8", "utf16_buff_to_utf8",
                ERR_WOULD_OVERFLOW, "utf8 size overflow"),
        };

    u64 totalBytes = bytesNeeded + 1;
    HeapStr out = (HeapStr)a->alloc(a, totalBytes);

    if (!out)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("utf8", "utf16_buff_to_utf8",
                ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    ptr = buff.units;
    i8 *write = out;

    while (ptr < end)
    {
        u16 first = (u16)*ptr++;
        u32 codepoint = 0;

        if (first >= 0xD800u && first <= 0xDBFFu)
        {
            u16 second = (u16)*ptr++;
            codepoint = 0x10000u + (((u32)first - 0xD800u) << 10) + ((u32)second - 0xDC00u);
        }
        else
        {
            codepoint = (u32)first;
        }

        if (codepoint <= 0x7Fu)
        {
            *write++ = (i8)codepoint;
        }
        else if (codepoint <= 0x7FFu)
        {
            *write++ = (i8)(0xC0u | (codepoint >> 6));
            *write++ = (i8)(0x80u | (codepoint & 0x3Fu));
        }
        else if (codepoint <= 0xFFFFu)
        {
            *write++ = (i8)(0xE0u | (codepoint >> 12));
            *write++ = (i8)(0x80u | ((codepoint >> 6) & 0x3Fu));
            *write++ = (i8)(0x80u | (codepoint & 0x3Fu));
        }
        else
        {
            *write++ = (i8)(0xF0u | (codepoint >> 18));
            *write++ = (i8)(0x80u | ((codepoint >> 12) & 0x3Fu));
            *write++ = (i8)(0x80u | ((codepoint >> 6) & 0x3Fu));
            *write++ = (i8)(0x80u | (codepoint & 0x3Fu));
        }
    }

    *write = 0;

    return (ResultOwnedStr){
        .value = out,
        .error = X_ERR_OK,
    };
}

static inline ResultOwnedStr utf16_to_utf8(Allocator *a, Utf16Str s)
{
    if (!a || !s)
        return (ResultOwnedStr){
            .value = NULL,
            .error = X_ERR_EXT("utf8", "utf16_to_utf8",
                ERR_INVALID_PARAMETER, "null argument"),
        };

    u64 units = 0;
    while (s[units])
        ++units;

    return utf16_buff_to_utf8(a, (Utf16Buff){
        .units = s,
        .size = units,
    });
}
