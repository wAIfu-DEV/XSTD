#pragma once

#include "xstd_core.h"

#define _DEF_VEC2_T(T) \
    typedef struct _vec2_##T \
    { \
        T x; \
        T y; \
    } Vec2##T

#define _DEF_VEC3_T(T) \
    typedef struct _vec3_##T \
    { \
        T x; \
        T y; \
        T z;  \
    } Vec3##T

_DEF_VEC2_T(i8);
_DEF_VEC2_T(u8);
_DEF_VEC2_T(i16);
_DEF_VEC2_T(u16);
_DEF_VEC2_T(i32);
_DEF_VEC2_T(u32);
_DEF_VEC2_T(i64);
_DEF_VEC2_T(u64);

_DEF_VEC2_T(f32);
_DEF_VEC2_T(f64);

_DEF_VEC3_T(i8);
_DEF_VEC3_T(u8);
_DEF_VEC3_T(i16);
_DEF_VEC3_T(u16);
_DEF_VEC3_T(i32);
_DEF_VEC3_T(u32);
_DEF_VEC3_T(i64);
_DEF_VEC3_T(u64);

_DEF_VEC3_T(f32);
_DEF_VEC3_T(f64);
