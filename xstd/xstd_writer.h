#pragma once

#include "xstd_core.h"
#include "xstd_error.h"
#include "xstd_alloc.h"
#include "xstd_string.h"

typedef struct _writer
{
    Error (*write)(void* writer, i8 byte);
    u64 writeHead;
    u64 writeEnd;
} Writer;

typedef struct _buff_writer
{
    Error (*write)(struct _buff_writer* w, i8 byte);
    u64 writeHead;
    u64 writeEnd;
    Buffer buff;
} BuffWriter;

typedef struct _grow_buff_writer
{
    Error (*write)(struct _grow_buff_writer* w, i8 byte);
    u64 writeHead;
    u64 writeEnd;
    HeapBuff buff;
    Allocator allocator;
} GrowBuffWriter;

typedef struct _grow_str_writer
{
    Error (*write)(struct _grow_str_writer* w, i8 byte);
    u64 writeHead;
    u64 writeEnd;
    HeapStr str;
    u32 strSize;
    Allocator allocator;
} GrowStrWriter;

typedef struct _result_buff_writer
{
    BuffWriter value;
    Error error;
} ResultBuffWriter;

typedef struct _result_grow_buff_writer
{
    GrowBuffWriter value;
    Error error;
} ResultGrowBuffWriter;

typedef struct _result_grow_str_writer
{
    GrowStrWriter value;
    Error error;
} ResultGrowStrWriter;


static Error buffwriter_write_byte(BuffWriter* bw, i8 byte)
{
    if (!bw)
        return X_ERR_EXT("writer", "buffwriter_write_byte", ERR_INVALID_PARAMETER, "null writer");

    if (bw->writeHead >= bw->writeEnd)
        return X_ERR_EXT("writer", "buffwriter_write_byte", ERR_WOULD_OVERFLOW, "tried writing past buffer end");

    bw->buff.bytes[bw->writeHead] = byte;
    ++bw->writeHead;
    return X_ERR_OK;
}

static Error growbuffwriter_write_byte(GrowBuffWriter* gbw, i8 byte)
{
    if (!gbw)
        return X_ERR_EXT("writer", "growbuffwriter_write_byte", ERR_INVALID_PARAMETER, "null writer");

    // Grow buffer
    if (gbw->writeHead >= gbw->writeEnd)
    {
        u64 newSize = gbw->buff.size * 2;

        if (newSize < gbw->buff.size)
            return X_ERR_EXT("writer", "growbuffwriter_write_byte", ERR_WOULD_OVERFLOW, "integer overflow with buffer size");

        i8* newBlock = (i8*)gbw->allocator.realloc(&gbw->allocator, gbw->buff.bytes, newSize);

        if (!newBlock)
            return X_ERR_EXT("writer", "growbuffwriter_write_byte", ERR_OUT_OF_MEMORY, "alloc failure");

        gbw->buff.bytes = newBlock;
        gbw->buff.size = newSize;
        gbw->writeEnd = newSize;
    }

    gbw->buff.bytes[gbw->writeHead] = byte;
    ++gbw->writeHead;
    return X_ERR_OK;
}

static Error growstrwriter_write_byte(GrowStrWriter* gsw, i8 byte)
{
    if (!gsw)
        return X_ERR_EXT("writer", "growstrwriter_write_byte", ERR_INVALID_PARAMETER, "null writer");

    // Grow buffer
    if (gsw->writeEnd == 0 || gsw->writeHead >= gsw->writeEnd - 1)
    {
        u64 newSize = gsw->strSize * 2;

        if (newSize < gsw->strSize)
            return X_ERR_EXT("writer", "growstrwriter_write_byte", ERR_WOULD_OVERFLOW, "integer overflow with buffer size");

        i8* newBlock = (i8*)gsw->allocator.realloc(&gsw->allocator, gsw->str, newSize);
        if (!newBlock)
            return X_ERR_EXT("writer", "growstrwriter_write_byte", ERR_OUT_OF_MEMORY, "alloc failure");

        gsw->str = newBlock;
        gsw->strSize = newSize;
        gsw->writeEnd = newSize;
    }

    gsw->str[gsw->writeHead] = byte;
    gsw->str[gsw->writeHead+1] = 0;
    ++gsw->writeHead;
    return X_ERR_OK;
}

static inline ResultBuffWriter buffwriter_init(Buffer buff)
{
    if (buff.bytes == NULL || buff.size == 0)
        return (ResultBuffWriter){
            .error = X_ERR_EXT("writer", "buffwriter_init", ERR_INVALID_PARAMETER, "null or empty buff"),
        };

    return (ResultBuffWriter){
        .value = (BuffWriter){
            .write = buffwriter_write_byte,
            .writeHead = 0,
            .writeEnd = buff.size,
            .buff = buff,
        },
        .error = X_ERR_OK,
    };
}

static inline ResultGrowBuffWriter growbuffwriter_init(Allocator alloc, u32 initSize)
{
    if (initSize == 0)
        return (ResultGrowBuffWriter){
            .error = X_ERR_EXT("writer", "growbuffwriter_init", ERR_INVALID_PARAMETER, "0 init size"),
        };

    i8* block = (i8*)alloc.alloc(&alloc, initSize);

    if (!block)
        return (ResultGrowBuffWriter){
            .error = X_ERR_EXT("writer", "growbuffwriter_init", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    return (ResultGrowBuffWriter){
        .value = (GrowBuffWriter){
            .write = growbuffwriter_write_byte,
            .writeHead = 0,
            .writeEnd = initSize,
            .buff = (HeapBuff){
                .bytes = block,
                .size = initSize,
            },
            .allocator = alloc,
        },
        .error = X_ERR_OK,
    };
}

static inline ResultGrowStrWriter growstrwriter_init(Allocator alloc, u32 initSize)
{
    if (initSize == 0)
        return (ResultGrowStrWriter){
            .error = X_ERR_EXT("writer", "growstrwriter_init", ERR_INVALID_PARAMETER, "0 init size"),
        };

    i8* block = (i8*)alloc.alloc(&alloc, initSize);

    if (!block)
        return (ResultGrowStrWriter){
            .error = X_ERR_EXT("writer", "growstrwriter_init", ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    block[0] = 0;

    return (ResultGrowStrWriter){
        .value = (GrowStrWriter){
            .write = growstrwriter_write_byte,
            .writeHead = 0,
            .writeEnd = initSize,
            .str = block,
            .strSize = initSize,
            .allocator = alloc,
        },
        .error = X_ERR_OK,
    };
}

static inline Error growbuffwriter_resize(GrowBuffWriter* gbw, u64 newSize)
{
    if (!gbw || newSize == 0)
        return X_ERR_EXT("writer", "growbuffwriter_resize", ERR_INVALID_PARAMETER, "null writer or 0 new size");

    i8* newBlock = (i8*)gbw->allocator.realloc(&gbw->allocator, gbw->buff.bytes, newSize);

    if (!newBlock)
        return X_ERR_EXT("writer", "growbuffwriter_resize", ERR_OUT_OF_MEMORY, "alloc failure");

    gbw->buff.bytes = newBlock;
    gbw->buff.size = newSize;
    gbw->writeEnd = newSize;
    gbw->writeHead = newSize < gbw->writeHead ? newSize : gbw->writeHead;
    return X_ERR_OK;
}

static inline Error growstrwriter_resize(GrowStrWriter* gbw, u64 newSize)
{
    if (!gbw || newSize == 0)
        return X_ERR_EXT("writer", "growstrwriter_resize", ERR_INVALID_PARAMETER, "null writer or 0 new size");

    i8* newBlock = (i8*)gbw->allocator.realloc(&gbw->allocator, gbw->str, newSize);

    if (!newBlock)
        return X_ERR_EXT("writer", "growstrwriter_resize", ERR_OUT_OF_MEMORY, "alloc failure");

    gbw->str = newBlock;
    gbw->strSize = newSize;
    gbw->writeEnd = newSize;
    gbw->writeHead = newSize < gbw->writeHead ? newSize : gbw->writeHead;
    return X_ERR_OK;
}

static inline Error writer_write_byte(Writer* w, i8 byte)
{
    if (!w)
        return X_ERR_EXT("writer", "writer_write_byte", ERR_INVALID_PARAMETER, "null writer");

    return w->write(w, byte);
}

static inline Error writer_write_null(Writer* w);

static inline Error writer_write_bytes(Writer* w, ConstBuff buff)
{
    if (!w || !buff.bytes)
        return X_ERR_EXT("writer", "writer_write_bytes", ERR_INVALID_PARAMETER, "null arg");

    for (u32 i = 0; i < buff.size && w->writeHead < w->writeEnd; ++i)
    {
        Error err = w->write(w, buff.bytes[i]);
        if (err.code != ERR_OK)
            return err;
    }
    return X_ERR_OK;
}

static inline Error writer_write_str(Writer* w, ConstStr text)
{
    if (!w)
        return X_ERR_EXT("writer", "writer_write_str", ERR_INVALID_PARAMETER, "null writer");

    if (!text)
        return writer_write_null(w);

    while (*text && w->writeHead < w->writeEnd)
    {
        Error err = w->write(w, *text);
        if (err.code != ERR_OK)
            return err;
        ++text;
    }
    return X_ERR_OK;
}

static inline Error writer_write_null(Writer* w)
{
    if (!w)
        return X_ERR_EXT("writer", "writer_write_null", ERR_INVALID_PARAMETER, "null writer");

    return writer_write_str(w, "(null)");
}

static inline Error writer_write_int(Writer* w, i64 i)
{
    if (!w)
        return X_ERR_EXT("writer", "writer_write_int", ERR_INVALID_PARAMETER, "null writer");

    char buf[20];
    i64 n = i;
    i16 idx = 0;
    Error err;

    if (n == 0)
    {
        err = writer_write_byte(w, '0');
        if (err.code != ERR_OK)
            return err;
        return X_ERR_OK;
    }

    if (n < 0)
    {
        err = writer_write_byte(w, '-');
        if (err.code != ERR_OK)
            return err;
        n = -n;
    }

    while (n != 0)
    {
        i16 d = (i16)(n % 10);
        buf[idx++] = digit_to_char(d);
        n /= 10;
    }

    for (i16 j = idx - 1; j >= 0; --j)
    {
        err = writer_write_byte(w, buf[j]);
        if (err.code != ERR_OK)
            return err;
    }
    return X_ERR_OK;
}

static inline Error writer_write_uint(Writer* w, u64 i)
{
    if (!w)
        return X_ERR_EXT("writer", "writer_write_uint", ERR_INVALID_PARAMETER, "null writer");

    char buf[20];
    i16 idx = 0;
    u64 n = i;
    Error err;

    if (n == 0)
    {
        err = writer_write_byte(w, '0');
        if (err.code != ERR_OK)
            return err;
        return X_ERR_OK;
    }

    while (n != 0)
    {
        i16 d = (i16)(n % 10);
        buf[idx++] = digit_to_char(d);
        n /= 10;
    }

    for (i16 j = idx - 1; j >= 0; --j)
    {
        err = writer_write_byte(w, buf[j]);
        if (err.code)
            return err;
    }
    return X_ERR_OK;
}

static inline Error writer_write_float(Writer* w, f64 flt, u64 precision)
{
    if (!w)
        return X_ERR_EXT("writer", "writer_write_float", ERR_INVALID_PARAMETER, "null writer");

    Error err;
    f64 absVal = (flt >= 0.0) ? flt : -flt;
    u64 intPart = (u64)absVal;
    f64 fracPart = absVal - (f64)intPart;

    f64 scale = 1.0;
    for (u64 i = 0; i < precision; ++i)
        scale *= 10.0;

    u64 fracInt = (precision > 0) ? (u64)(fracPart * scale + 0.5) : 0;

    if (fracInt >= (u64)scale && precision > 0)
    {
        fracInt -= (u64)scale;
        ++intPart;
    }

    if (flt < 0.0)
    {
        err = writer_write_byte(w, '-');
        if (err.code != ERR_OK)
            return err;
    }

    err = writer_write_uint(w, intPart);
    if (err.code != ERR_OK)
        return err;

    if (precision == 0)
        return X_ERR_OK;

    err = writer_write_byte(w, '.');
    if (err.code != ERR_OK)
        return err;

    u64 divisor = (u64)(scale / 10.0);
    for (u64 i = 0; i < precision; ++i)
    {
        if (divisor == 0)
            divisor = 1;

        u64 digit = fracInt / divisor;
        err = writer_write_byte(w, digit_to_char((i16)digit));
        if (err.code != ERR_OK)
            return err;
        fracInt -= digit * divisor;
        divisor /= 10;
    }
    return X_ERR_OK;
}
