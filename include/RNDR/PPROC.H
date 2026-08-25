/* Copyright (c) 2026 Burak Yazar */

#ifndef PPROC_H
#define PPROC_H

#include "../CORE/FIXED.H"
#include "../CORE/TYPES/WORLD.H"

/* ---- Post process module ----
 * Light rays using depth buffer.
 */

/* Tuning Constants */

#define DECAY 58982 /* 0.90 in Q16.16 */

/**
 * @brief Computes illuminated color.
 * @param base_color input color index
 * @param fx_intensity intensity level in fixed point: 0(brightest),31(darkest)
 * @return illuminated color's resulting index.
 */
u8 pproc_palette_add(u8 base_color, i32 fx_intensity);

/**
 * @brief Computes and overdraws light rays in planar memory
 * @param dst Destination color buffer
 * @param num_lights Number of lights to compute
 * @param light_screen_pos Array of lights in world
 * @param light_cam_z Array of lights' camera-space Z coordinates
 * @param fx_radius Radial distance of rays from light source
 */
void pproc_apply_light_rays(u8 *dst, u32 num_lights, const Vec2 *light_screen_pos, const i32 *light_cam_z, i32 fx_radius);

#define PPROC_FLAG_DOF (1 << 0)
#define PPROC_FLAG_FOG (1 << 1)

typedef struct PProcParams_s
{
    u32 flags;
    i32 dof_focus_z;
    i32 dof_focus_range;
    int dof_max_blur;
} PProcParams;

/**
 * @brief Applies selected post-processing effects in a single optimized pass
 * @param dst Destination color buffer
 * @param params Post-processing parameters
 */
void pproc_apply(u8 *dst, const PProcParams *params);

#endif /* PPROC_H */