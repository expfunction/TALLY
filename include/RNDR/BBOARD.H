/* Copyright (c) 2026 Burak Yazar */

#ifndef BBOARD_H
#define BBOARD_H

#include "../CORE/TYPES/WORLD.H"
#include "../CORE/TYPES/GFX.H"
#include "../RNDR/CAMER.H"

/* Screen-space descriptor built once per visible billboard before plane loop */
typedef struct BbDesc_s
{
    int x0, y0, x1, y1; /* screen bounding box (integer pixels, x1/y1 exclusive) */
    i32 du, dv;         /* source texels per screen pixel (Q16.16) */
    const u8 *tex;
    int tex_w, tex_h; /* texture dimensions (power-of-2) */
    u32 oow;          /* 1/cam_z as u32 for depth compare (larger = closer) */
    int color_key;    /* palette index to treat as transparent, or -1 */
    u8 alpha;         /* 0 = invisible, 255 = fully opaque */
    int ll, fl;       /* light level 0..31, fog level 0..31 */
} BbDesc;

/**
 * @brief Render world billboards as textured quads that always face the camera.
 *
 * All visible billboards are projected to screen-space axis-aligned rectangles,
 * then flushed plane by plane (4 outp calls total regardless of billboard count).
 * Per-scanline ClipSpans bounds replace the coarser ClipRect, so portal geometry
 * is respected at pixel precision.
 *
 * @param buf        VRAM base pointer
 * @param cam        Camera for world-to-screen transform
 * @param num_bbs    Number of billboards in the array
 * @param bbs        WorldBillboard array
 * @param num_lights Number of world point lights
 * @param lights     Point light array
 * @param spans      Per-scanline clip span table (portal bounds)
 */
void bboard_render_spans(u8 *buf, const Camera *cam,
                         int num_bbs, const WorldBillboard *bbs,
                         int num_lights, const PointLight *lights,
                         const ClipSpans *spans);

#endif /* BBOARD_H */
