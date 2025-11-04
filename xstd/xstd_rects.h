#pragma once

#include "xstd_core.h"

#define __DEF_RECT_T(T) \
    typedef struct _rect_##T \
    { \
        T x; \
        T y; \
        T width; \
        T height; \
    } Rect##T

__DEF_RECT_T(i8);
__DEF_RECT_T(u8);
__DEF_RECT_T(i16);
__DEF_RECT_T(u16);
__DEF_RECT_T(i32);
__DEF_RECT_T(u32);
__DEF_RECT_T(i64);
__DEF_RECT_T(u64);

__DEF_RECT_T(f32);
__DEF_RECT_T(f64);
