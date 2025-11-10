#pragma once

#include "xstd_core.h"

static inline void mem_copy(void *dst, const void *src, u64 size)
{
    if (!dst || !src || size == 0)
        return;

    u8 *d = (u8*)dst;
    const u8 *s = (const u8*)src;

    if (d == s)
        return;

    while (((uPtr)d & (sizeof(u64) - 1)) && size != 0)
    {
        *d++ = *s++;
        --size;
    }

    if (((uPtr)s & (sizeof(u64) - 1)) == 0)
    {
        const u64 *src64 = (const u64*)s;
        u64 *dst64 = (u64*)d;

        while (size >= sizeof(u64))
        {
            *dst64++ = *src64++;
            size -= sizeof(u64);
        }

        d = (u8*)dst64;
        s = (const u8*)src64;
    }

    while (size != 0)
    {
        *d++ = *s++;
        --size;
    }
}
