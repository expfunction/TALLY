/* Copyright (c) 2026 Burak Yazar */

#ifndef MATH_TYPES_H
#define MATH_TYPES_H

/* Primitive integer and float aliases */
typedef unsigned char u8;
typedef signed char i8;
typedef unsigned short u16;
typedef signed short i16;
typedef unsigned long u32;
typedef signed long i32;
typedef unsigned long long u64;
typedef signed long long i64;
typedef float f32;
typedef double f64;

/* CMS mesh flags */
#define CVG_HAS_UV 0x01
#define CVG_HAS_NORMALS 0x02
#define CVG_HAS_VCOLORS 0x04

/* Boolean type */
#ifndef __bool_true_false_are_defined
typedef enum bool_e
{
    false,
    true
} bool;
#endif

/* DPMI interface aliases (DJGPP) */
#ifdef __DJGPP__
#include <dpmi.h>
#include <go32.h>
typedef __dpmi_regs DPMIRegs;
typedef __dpmi_raddr DPMIRAddr;
typedef __dpmi_paddr DPMIPAddr;
typedef __dpmi_meminfo DPMIMemInfo;
typedef __dpmi_version_ret DPMIVersionRet;
typedef __dpmi_memory_info DPMIMemoryInfo;
typedef __dpmi_free_mem_info DPMIFreeMemInfo;
typedef __dpmi_callback_info DPMICallbackInfo;
#endif

/* 4x4 Q16.16 matrix */
typedef struct Mat4x4_s
{
    i32 m[4][4];
} Mat4x4;

/* Integer 2D vector */
typedef struct Vec2_s
{
    i32 x, y;
} Vec2;

/* Q16.16 3D vector */
typedef struct Vec3_s
{
    i32 x, y, z;
} Vec3;

/* Q16.16 4D vector */
typedef struct Vec4_s
{
    i32 x, y, z, w;
} Vec4;

/* Real-time game clock */
typedef struct GameClock_s
{
    i64 prev, curr; /* raw PIT tick counters */
    i32 dt;         /* frame delta time Q16.16 seconds */
    i32 dt_ms;      /* frame delta time in milliseconds (integer) */
    i32 fps;        /* frames per second (integer) */
    u32 frame;      /* monotonic frame counter */
} GameClock;

#endif /* MATH_TYPES_H */
