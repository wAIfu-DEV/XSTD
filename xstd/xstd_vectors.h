#pragma once

#include "xstd_core.h"

#define __DEF_VEC2_T(T) \
    typedef struct _vec2_##T \
    { \
        T x; \
        T y; \
    } Vec2##T;

#define __DEF_VEC3_T(T) \
    typedef struct _vec3_##T \
    { \
        T x; \
        T y; \
        T z;  \
    } Vec3##T;

__DEF_VEC2_T(i8);
__DEF_VEC2_T(u8);
__DEF_VEC2_T(i16);
__DEF_VEC2_T(u16);
__DEF_VEC2_T(i32);
__DEF_VEC2_T(u32);
__DEF_VEC2_T(i64);
__DEF_VEC2_T(u64);

__DEF_VEC2_T(f32);
__DEF_VEC2_T(f64);

__DEF_VEC3_T(i8);
__DEF_VEC3_T(u8);
__DEF_VEC3_T(i16);
__DEF_VEC3_T(u16);
__DEF_VEC3_T(i32);
__DEF_VEC3_T(u32);
__DEF_VEC3_T(i64);
__DEF_VEC3_T(u64);

__DEF_VEC3_T(f32);
__DEF_VEC3_T(f64);
