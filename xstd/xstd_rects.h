#pragma once

#include "xstd_core.h"

#define _DEF_RECT_T(T) \
    typedef struct _rect_##T \
    { \
        T x; \
        T y; \
        T width; \
        T height; \
    } Rect##T

_DEF_RECT_T(i8);
_DEF_RECT_T(u8);
_DEF_RECT_T(i16);
_DEF_RECT_T(u16);
_DEF_RECT_T(i32);
_DEF_RECT_T(u32);
_DEF_RECT_T(i64);
_DEF_RECT_T(u64);

_DEF_RECT_T(f32);
_DEF_RECT_T(f64);
