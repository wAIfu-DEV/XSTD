#pragma once

#include "xstd_core.h"
#include "xstd_error.h"
#include "xstd_alloc.h"
#include "xstd_alloc_default.h"
#include "xstd_string.h"

typedef struct _writer
{
    void *_internalState;
    Error (*const write)(struct _writer *writer, i8 byte);
    void (*const deinit)(struct _writer *writer);
} Writer;

typedef struct _result_writer
{
    Writer value;
    Error error;
} ResultWriter;

typedef struct _buff_writer_state
{
    Buffer *buff;
    Allocator stateAlloc;
    u64 writeHead;
    u64 writeEnd;
} _BuffWriterState;

typedef struct _grow_buff_writer_state
{
    HeapBuff buff;
    Allocator allocator;
    u64 writeHead;
    u64 writeEnd;
} _GrowBuffWriterState;

typedef struct _grow_str_writer_state
{
    HeapStr str;
    u32 strSize;
    Allocator allocator;
    u64 writeHead;
    u64 writeEnd;
} _GrowStrWriterState;

static inline void writer_deinit(Writer *w)
{
    if (!w)
        return;

    if (w->deinit)
        w->deinit(w);
}

static Error _buffwriter_write(Writer *writer, i8 byte)
{
    if (!writer)
        return X_ERR_EXT("writer", "buffwriter_write",
            ERR_INVALID_PARAMETER, "null writer");

    _BuffWriterState *state = (_BuffWriterState *)writer->_internalState;
    if (!state || !state->buff->bytes)
        return X_ERR_EXT("writer", "buffwriter_write",
            ERR_INVALID_PARAMETER, "invalid state");

    if (state->writeHead >= state->writeEnd)
        return X_ERR_EXT("writer", "buffwriter_write",
            ERR_WOULD_OVERFLOW, "tried writing past buffer end");

    state->buff->bytes[state->writeHead++] = byte;
    return X_ERR_OK;
}

static void _buffwriter_deinit(Writer *writer)
{
    if (!writer || !writer->_internalState)
        return;

    _BuffWriterState *state = (_BuffWriterState *)writer->_internalState;
    if (!state)
        return; // failure case, we may drop data

    Allocator *a = &state->stateAlloc;

    a->free(a, state);
    state = NULL;
    writer->_internalState = NULL;
}

static inline ResultWriter buffwriter_init(Allocator *a, Buffer *buff)
{
    if (!buff || !buff->bytes || buff->size == 0)
        return (ResultWriter){
            .error = X_ERR_EXT("writer", "buffwriter_init",
                ERR_INVALID_PARAMETER, "null or empty buff"),
        };

    _BuffWriterState *state = (_BuffWriterState *)a->alloc(a, sizeof(_BuffWriterState));
    if (!state)
        return (ResultWriter){
            .error = X_ERR_EXT("writer", "buffwriter_init",
                ERR_OUT_OF_MEMORY, "alloc failure"),
        };

    *state = (_BuffWriterState){
        .buff = buff,
        .stateAlloc = *a,
        .writeHead = 0,
        .writeEnd = buff->size,
    };

    return (ResultWriter){
        .value = (Writer){
            ._internalState = state,
            .write = _buffwriter_write,
            .deinit = _buffwriter_deinit,
        },
        .error = X_ERR_OK,
    };
}

static inline Error buffwriter_reset(Writer *writer)
{
    if (!writer)
        return X_ERR_EXT("writer", "buffwriter_reset",
            ERR_INVALID_PARAMETER, "null writer");

    _BuffWriterState *state = (_BuffWriterState *)writer->_internalState;
    if (!state)
        return X_ERR_EXT("writer", "buffwriter_reset",
            ERR_INVALID_PARAMETER, "invalid state");

    state->writeHead = 0;
    return X_ERR_OK;
}

static inline void buffwriter_deinit(Writer *writer)
{
    writer_deinit(writer);
}

static Error _growbuffwriter_resize(Writer *writer, u64 newSize);

static Error _growbuffwriter_write(Writer *writer, i8 byte)
{
    if (!writer)
        return X_ERR_EXT("writer", "growbuffwriter_write", ERR_INVALID_PARAMETER, "null writer");

    _GrowBuffWriterState *state = (_GrowBuffWriterState *)writer->_internalState;
    if (!state || !state->buff.bytes)
        return X_ERR_EXT("writer", "growbuffwriter_write", ERR_INVALID_PARAMETER, "invalid state");

    if (state->writeHead >= state->writeEnd)
    {
        u64 currentSize = state->buff.size ? state->buff.size : 1;
        u64 newSize = currentSize * 2;

        if (newSize < currentSize)
            return X_ERR_EXT("writer", "growbuffwriter_write", ERR_WOULD_OVERFLOW, "integer overflow with buffer size");

        Error resizeErr = _growbuffwriter_resize(writer, newSize);
        if (resizeErr.code != ERR_OK)
            return resizeErr;
    }

    state->buff.bytes[state->writeHead++] = byte;
    return X_ERR_OK;
}

static Error _growbuffwriter_resize(Writer *writer, u64 newSize)
{
    if (!writer)
        return X_ERR_EXT("writer", "growbuffwriter_resize", ERR_INVALID_PARAMETER, "null writer");

    _GrowBuffWriterState *state = (_GrowBuffWriterState *)writer->_internalState;
    if (!state || newSize == 0)
        return X_ERR_EXT("writer", "growbuffwriter_resize", ERR_INVALID_PARAMETER, "invalid state or size");

    i8 *newBlock = (i8*)state->allocator.realloc(&state->allocator, state->buff.bytes, newSize);
    if (!newBlock)
        return X_ERR_EXT("writer", "growbuffwriter_resize", ERR_OUT_OF_MEMORY, "alloc failure");

    state->buff.bytes = newBlock;
    state->buff.size = newSize;
    state->writeEnd = newSize;
    if (state->writeHead > newSize)
        state->writeHead = newSize;

    return X_ERR_OK;
}

static void _growbuffwriter_deinit(Writer *writer)
{
    if (!writer || !writer->_internalState)
        return;

    _GrowBuffWriterState *state = (_GrowBuffWriterState *)writer->_internalState;
    if (!state)
        return;

    Allocator *alloc = &state->allocator;

    if (state->buff.bytes)
        alloc->free(alloc, state->buff.bytes);

    state->buff.bytes = NULL;

    alloc->free(alloc, state);
    writer->_internalState = NULL;
    state = NULL;
}

static inline ResultWriter growbuffwriter_init(Allocator alloc, u32 initSize)
{
    if (initSize == 0)
        return (ResultWriter){
            .error = X_ERR_EXT("writer", "growbuffwriter_init",
                ERR_INVALID_PARAMETER, "0 init size"),
        };

    _GrowBuffWriterState *state = (_GrowBuffWriterState *)alloc.alloc(&alloc, sizeof(_GrowBuffWriterState));
    if (!state)
        return (ResultWriter){
            .error = X_ERR_EXT("writer", "growbuffwriter_init",
                ERR_OUT_OF_MEMORY, "state alloc failure"),
        };

    i8 *block = (i8 *)alloc.alloc(&alloc, initSize);
    if (!block)
    {
        alloc.free(&alloc, state);
        return (ResultWriter){
            .error = X_ERR_EXT("writer", "growbuffwriter_init",
                ERR_OUT_OF_MEMORY, "buff alloc failure"),
        };
    }

    *state = (_GrowBuffWriterState){
        .buff = (HeapBuff){
            .bytes = block,
            .size = initSize,
        },
        .allocator = alloc,
        .writeHead = 0,
        .writeEnd = initSize,
    };

    return (ResultWriter){
        .value = (Writer){
            ._internalState = state,
            .write = _growbuffwriter_write,
            .deinit = _growbuffwriter_deinit,
        },
        .error = X_ERR_OK,
    };
}

static inline Error growbuffwriter_reset(Writer *writer, u64 newSize)
{
    if (!writer)
        return X_ERR_EXT("writer", "growbuffwriter_reset",
            ERR_INVALID_PARAMETER, "null arg");

    _GrowBuffWriterState *state = (_GrowBuffWriterState*)writer->_internalState;
    if (!state)
        return X_ERR_EXT("writer", "growbuffwriter_reset",
            ERR_INVALID_PARAMETER, "invalid state");
    
    Allocator *a = &state->allocator;

    if (state->buff.bytes != NULL)
        a->free(a, state->buff.bytes);
    
    state->buff = (HeapBuff){0};
    state->writeHead = 0;
    state->writeEnd = 0;

    i8* newBlock = a->alloc(a, newSize);
    if (!newBlock)
        return X_ERR_EXT("writer", "growbuffwriter_reset",
            ERR_OUT_OF_MEMORY, "alloc failure");
    
    state->buff.bytes = newBlock;
    state->buff.size = newSize;
    return X_ERR_OK;
}

/**
 * @brief Get ownership of the underlying buffer of the GrowBuffWriter. Invalidates writer.
 * 
 * @warning Call to this function invalidates the passed caller until growbuffwriter_reset
 * is called.
 * 
 * @param writer 
 * @return ResultOwnedBuff 
 */
static inline ResultOwnedBuff growbuffwriter_data(Writer *writer)
{
    if (!writer)
        return (ResultOwnedBuff){
            .error = X_ERR_EXT("writer", "growbuffwriter_data", ERR_INVALID_PARAMETER, "null arg"),
        };

    _GrowBuffWriterState *state = (_GrowBuffWriterState *)writer->_internalState;
    if (!state || !state->buff.bytes)
        return (ResultOwnedBuff){
            .error = X_ERR_EXT("writer", "growbuffwriter_data", ERR_INVALID_PARAMETER, "invalid state"),
        };
    
    HeapBuff resBuff = (HeapBuff){
        .bytes = state->buff.bytes,
        .size = state->writeHead,
    };

    // Invalidate writer
    state->buff.bytes = NULL;
    state->writeHead = 0;
    state->writeEnd = 0;

    return (ResultOwnedBuff){
        .value = resBuff,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Copies underlying buffer of the GrowBuffWriter.
 * unlike growbuffwriter_data, the writer stays in a valid state.
 * 
 * @param writer 
 * @return ResultOwnedBuff 
 */
static inline ResultOwnedBuff growbuffwriter_data_copy(Writer *writer)
{
    if (!writer)
        return (ResultOwnedBuff){
            .error = X_ERR_EXT("writer", "growbuffwriter_data_copy",
                ERR_INVALID_PARAMETER, "null arg"),
        };

    _GrowBuffWriterState *state = (_GrowBuffWriterState *)writer->_internalState;
    if (!state || !state->buff.bytes)
        return (ResultOwnedBuff){
            .error = X_ERR_EXT("writer", "growbuffwriter_data_copy",
                ERR_INVALID_PARAMETER, "invalid state"),
        };

    Allocator *a = &state->allocator;

    u64 buffSize = state->writeHead;
    i8* newBlock = (i8*)a->alloc(a, buffSize);
    if (!newBlock)
        return (ResultOwnedBuff){
            .error = X_ERR_EXT("writer", "growbuffwriter_data_copy",
                ERR_OUT_OF_MEMORY, "alloc failure"),
        };
    
    mem_copy(newBlock, state->buff.bytes, buffSize);

    return (ResultOwnedBuff){
        .value = (HeapBuff){
            .bytes = newBlock,
            .size = buffSize,
        },
        .error = X_ERR_OK,
    };
}

static inline void growbuffwriter_deinit(Writer *writer)
{
    writer_deinit(writer);
}

static Error _growstrwriter_resize(Writer *writer, u64 newSize);

static Error _growstrwriter_write(Writer *writer, i8 byte)
{
    if (!writer)
        return X_ERR_EXT("writer", "growstrwriter_write",
            ERR_INVALID_PARAMETER, "null writer");

    _GrowStrWriterState *state = (_GrowStrWriterState *)writer->_internalState;
    if (!state || !state->str)
        return X_ERR_EXT("writer", "growstrwriter_write",
            ERR_INVALID_PARAMETER, "invalid state");

    if (state->writeEnd == 0 || state->writeHead >= state->writeEnd - 1)
    {
        u64 currentSize = state->strSize ? (u64)state->strSize : 1;
        u64 newSize = currentSize * 2;

        if (newSize < currentSize)
            return X_ERR_EXT("writer", "growstrwriter_write",
                ERR_WOULD_OVERFLOW, "integer overflow with buffer size");

        Error resizeErr = _growstrwriter_resize(writer, newSize);
        if (resizeErr.code != ERR_OK)
            return resizeErr;
    }

    state->str[state->writeHead] = byte;
    state->str[state->writeHead + 1] = 0;
    ++state->writeHead;
    return X_ERR_OK;
}

static Error _growstrwriter_resize(Writer *writer, u64 newSize)
{
    if (!writer)
        return X_ERR_EXT("writer", "growstrwriter_resize", ERR_INVALID_PARAMETER, "null writer");

    _GrowStrWriterState *state = (_GrowStrWriterState *)writer->_internalState;
    if (!state || newSize == 0)
        return X_ERR_EXT("writer", "growstrwriter_resize", ERR_INVALID_PARAMETER, "invalid state or size");

    char *newBlock = (char*)state->allocator.realloc(&state->allocator, state->str, newSize);
    if (!newBlock)
        return X_ERR_EXT("writer", "growstrwriter_resize", ERR_OUT_OF_MEMORY, "alloc failure");

    state->str = newBlock;
    state->strSize = (u32)newSize;
    state->writeEnd = newSize;
    if (state->writeHead >= newSize)
        state->writeHead = (newSize > 0) ? newSize - 1 : 0;
    state->str[state->writeHead] = 0;

    return X_ERR_OK;
}

static void _growstrwriter_deinit(Writer *writer)
{
    if (!writer || !writer->_internalState)
        return;

    _GrowStrWriterState *state = (_GrowStrWriterState *)writer->_internalState;
    if (!state)
        return;

    Allocator *alloc = &state->allocator;
    if (state->str)
        alloc->free(alloc, state->str);

    state->str = NULL;
    state->strSize = 0;
    state->writeHead = 0;
    state->writeEnd = 0;
    writer->_internalState = NULL;
    alloc->free(alloc, state);
}

static inline ResultWriter growstrwriter_init(Allocator alloc, u32 initSize)
{
    if (initSize == 0)
        return (ResultWriter){
            .error = X_ERR_EXT("writer", "growstrwriter_init",
                ERR_INVALID_PARAMETER, "0 init size"),
        };

    _GrowStrWriterState *state = (_GrowStrWriterState *)alloc.alloc(&alloc, sizeof(_GrowStrWriterState));
    if (!state)
        return (ResultWriter){
            .error = X_ERR_EXT("writer", "growstrwriter_init",
                ERR_OUT_OF_MEMORY, "state alloc failure"),
        };

    char *block = (char*)alloc.alloc(&alloc, initSize);
    if (!block)
    {
        alloc.free(&alloc, state);
        return (ResultWriter){
            .error = X_ERR_EXT("writer", "growstrwriter_init",
                ERR_OUT_OF_MEMORY, "str alloc failure"),
        };
    }

    block[0] = 0;

    *state = (_GrowStrWriterState){
        .str = block,
        .strSize = initSize,
        .allocator = alloc,
        .writeHead = 0,
        .writeEnd = initSize,
    };

    return (ResultWriter){
        .value = (Writer){
            ._internalState = state,
            .write = _growstrwriter_write,
            .deinit = _growstrwriter_deinit,
        },
        .error = X_ERR_OK,
    };
}

/**
 * @brief Gain ownership of the underlying string of the GrowStrWriter. Invalidates the writer.
 * 
 * @param writer 
 * @param outStr 
 * @return ResultOwnedStr 
 */
static inline ResultOwnedStr growstrwriter_data(Writer *writer)
{
    if (!writer)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("writer", "growstrwriter_data",
                ERR_INVALID_PARAMETER, "null arg"),
        };

    _GrowStrWriterState *state = (_GrowStrWriterState *)writer->_internalState;
    if (!state || !state->str)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("writer", "growstrwriter_data",
                ERR_INVALID_PARAMETER, "invalid state"),
        };
    
    char* newStr = state->str;

    state->str = NULL;
    state->strSize = 0;
    state->writeHead = 0;
    state->writeEnd = 0;

    return (ResultOwnedStr){
        .value = newStr,
        .error = X_ERR_OK,
    };
}

/**
 * @brief Create a copy of the GrowStrWriter's underlying string.
 * Unlike growstrwriter_data, writer will still be valid.
 * 
 * @param writer 
 * @return ResultOwnedStr 
 */
static inline ResultOwnedStr growstrwriter_data_copy(Writer *writer)
{
    if (!writer)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("writer", "growstrwriter_data_copy",
                ERR_INVALID_PARAMETER, "null arg"),
        };

    _GrowStrWriterState *state = (_GrowStrWriterState *)writer->_internalState;
    if (!state || !state->str)
        return (ResultOwnedStr){
            .error = X_ERR_EXT("writer", "growstrwriter_data_copy",
                ERR_INVALID_PARAMETER, "invalid state"),
        };

    Allocator* a = &state->allocator;
    
    u64 strSize = state->writeHead;
    char* newStr = a->alloc(a, strSize + 1);

    mem_copy(newStr, state->str, strSize + 1);

    return (ResultOwnedStr){
        .value = newStr,
        .error = X_ERR_OK,
    };
}

static inline Error growstrwriter_reset(Writer *writer, u64 newSize)
{
    if (newSize == 0)
        return X_ERR_EXT("writer", "growstrwriter_init",
                ERR_INVALID_PARAMETER, "0 init size");

    if (!writer)
        return X_ERR_EXT("writer", "growstrwriter_reset",
            ERR_INVALID_PARAMETER, "null writer");

    _GrowStrWriterState *state = (_GrowStrWriterState *)writer->_internalState;
    if (!state)
        return X_ERR_EXT("writer", "growstrwriter_reset",
            ERR_INVALID_PARAMETER, "invalid state");

    Allocator* a = &state->allocator;

    state->str = NULL;
    state->strSize = 0;

    state->writeHead = 0;
    state->writeEnd = 0;

    char* newBlock = (char*)a->alloc(a, newSize);
    if (!newBlock)
        return X_ERR_EXT("writer", "growbuffwriter_reset",
            ERR_OUT_OF_MEMORY, "alloc failure");
    
    if (newSize > 0) {
        newBlock[0] = 0;
    }
    
    state->str = newBlock;
    state->strSize = newSize;
    state->writeEnd = newSize;
    return X_ERR_OK;
}

static inline void growstrwriter_deinit(Writer *writer)
{
    writer_deinit(writer);
}

static inline Error writer_write_byte(Writer *w, i8 byte)
{
    if (!w)
        return X_ERR_EXT("writer", "writer_write_byte",
            ERR_INVALID_PARAMETER, "null writer");

    return w->write(w, byte);
}

static inline Error writer_write_null(Writer *w);

static inline Error writer_write_bytes(Writer *w, ConstBuff buff)
{
    if (!w || !buff.bytes)
        return X_ERR_EXT("writer", "writer_write_bytes",
            ERR_INVALID_PARAMETER, "null arg");

    for (u32 i = 0; i < buff.size; ++i)
    {
        Error err = w->write(w, buff.bytes[i]);
        if (err.code != ERR_OK)
            return err;
    }
    return X_ERR_OK;
}

static inline Error writer_write_str(Writer *w, ConstStr text)
{
    if (!w)
        return X_ERR_EXT("writer", "writer_write_str",
            ERR_INVALID_PARAMETER, "null writer");

    if (!text)
        return writer_write_null(w);

    while (*text)
    {
        Error err = w->write(w, *text);
        if (err.code != ERR_OK)
            return err;
        ++text;
    }
    return X_ERR_OK;
}

static inline Error writer_write_null(Writer *w)
{
    if (!w)
        return X_ERR_EXT("writer", "writer_write_null",
            ERR_INVALID_PARAMETER, "null writer");

    return writer_write_str(w, "(null)");
}

static inline Error writer_write_int(Writer *w, i64 i)
{
    if (!w)
        return X_ERR_EXT("writer", "writer_write_int",
            ERR_INVALID_PARAMETER, "null writer");

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

static inline Error writer_write_uint(Writer *w, u64 i)
{
    if (!w)
        return X_ERR_EXT("writer", "writer_write_uint",
            ERR_INVALID_PARAMETER, "null writer");

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

static inline Error writer_write_float(Writer *w, f64 flt, u64 precision)
{
    if (!w)
        return X_ERR_EXT("writer", "writer_write_float",
            ERR_INVALID_PARAMETER, "null writer");

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

