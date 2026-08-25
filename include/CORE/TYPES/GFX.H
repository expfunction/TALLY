/* Copyright (c) 2026 Burak Yazar */

#ifndef GFX_TYPES_H
#define GFX_TYPES_H

#include "MATH.H"

/* 8-bit surface (back buffer + page) */
typedef struct Surface8_s
{
    u32 w, h;
    u8 *back;
    u8 page;
} Surface8;

/* 8-bit paletted sprite */
typedef struct Sprite_s
{
    bool planar;
    u8 *name;
    u32 w, h;
    u8 *pix;
    char *path;
} Sprite;

/* Screen-space clipping rectangle */
typedef struct ClipRect_s
{
    i32 minx, miny;
    i32 maxx, maxy;
} ClipRect;

/* Perspective-transformed vertex */
typedef struct V2PT_s
{
    Vec2 screen_pos;   /* screen-space pixel coordinates (integer) */
    Vec2 uv;           /* texel coordinates Q16.16 */
    i32 uow, vow, oow; /* perspective-correct u/w, v/w, 1/w Q16.16 */
    i32 uow2, vow2;    /* perspective-correct blend-map u/w, v/w Q16.16 */
    i32 uow3, vow3;    /* texture 1 */
    i32 uow4, vow4;    /* texture 2 */
    i32 uow5, vow5;    /* texture 3 */
    i32 uow6, vow6;    /* texture 4 */
    i32 uow7, vow7;    /* texture 5 */
    i32 uow8, vow8;    /* texture 6 */
    i32 uow9, vow9;    /* texture 7 */
    i32 light_level;   /* shading intensity (integer, 0..255) */
    i32 spec_level;    /* Speculer intensity */
    u8 light_color;    /* Dominant light color (LightColor enum) */
#ifdef PLATFORM_WIN64
    /* PBR interpolation attributes */
    float fwpos_x_ow, fwpos_y_ow, fwpos_z_ow;
    float fnx_ow, fny_ow, fnz_ow;
    float ftx_ow, fty_ow, ftz_ow;
    float foow;
    float fuow, fvow;
#endif
} V2PT;

/* Screen-space quad (up to 8 verts after near-plane clip) */
#define MAX_SCREENQUAD_VERTS 8
typedef struct ScreenQuad_s
{
    i32 sx[MAX_SCREENQUAD_VERTS];
    i32 sy[MAX_SCREENQUAD_VERTS];
    int valid_count;
} ScreenQuad;

/* Per-scanline clip span table — exact convex portal outline */
#define SCREEN_W 320
#define SCREEN_H 240
typedef struct ClipSpans_s
{
    int miny;            /* first covered scanline (integer) */
    int maxy;            /* one past last covered scanline (exclusive) */
    i32 left[SCREEN_H];  /* left x bound at scanline y (Q16.16) */
    i32 right[SCREEN_H]; /* right x bound at scanline y (Q16.16) */
} ClipSpans;

/**
 * @brief Build a full-rect ClipSpans from a ClipRect (constant x per row)
 * @param r
 * @param out
 */
void clipspans_from_cliprect(const ClipRect *r, ClipSpans *out);
/**
 * @brief Compute bounding ClipRect from a ClipSpans (conservative AABB)
 * @param s
 */
ClipRect clipspans_to_cliprect(const ClipSpans *s);

#endif /* GFX_TYPES_H */
