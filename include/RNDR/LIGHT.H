/* Copyright (c) 2026 Burak Yazar */

#ifndef LIGHT_H
#define LIGHT_H

#include "CORE/TYPES/MATH.H"
#include "CORE/FIXED.H"
#include "CORE/TYPES/WORLD.H"

/* Number of shades for light: 0 = full bright , NUM_LIGHT_LEVELS-1 = black */
#define NUM_LIGHT_LEVELS 32

/* Maximum light-level contribution from the sun (0=bright, 31=dark scale).
 * 16 means a surface directly facing a noon sun reaches level 31-16=15 (mid bright). */
#define SUN_MAX_CONTRIB 16

/* Forward decleration (!) */
typedef struct PointLight_s PointLight;

/* Palette is organized as 8 ramps of 32 entries each (dark->bright).
   The colormap builder constrains nearest-color search to the same ramp,
   preventing cross-hue shifts when darkening. */
#define PAL_RAMP_SIZE 32
#define PAL_NUM_RAMPS 8

#define NUM_LIGHT_COLORS 5

/* Colormap table: colormap[color][level][palette_index] = shaded palette color */
extern u8 g_color_maps[NUM_LIGHT_COLORS][NUM_LIGHT_LEVELS][256];
/* Alias for backward compatibility */
#define g_colormap (g_color_maps[0])

extern u8 g_specmap[NUM_LIGHT_LEVELS][256];

#ifdef PLATFORM_WIN64
/* Flattened light and specular map for zero-dependency WIN64 pipeline.
   Indexed by [color][sl][ll][raw_texel] */
extern u8 g_flat_light_spec[NUM_LIGHT_COLORS][NUM_LIGHT_LEVELS][NUM_LIGHT_LEVELS][256];
void flatten_light_spec_maps(void);
#endif /**                                                                                 \
        * @brief Build colormap from current VGA palette. Call once after loading palette. \
        * @param pal Pointer to 768-byte (DAC, 256*RGB, 6-bit values 0..63) palette.       \
        */
void light_build_colormap(const u8 *pal);
void light_build_specmap(const u8 *pal);

/**
 * @brief Computes light intensity(level) and specular using Q16.16 distance and emit_level.
 * @param dist_fx
 * @param emit_level
 * @return
 */
int light_level_from_dist(i32 dist_fx, int emit_level);

int vertex_light(const Vec4 *wp, const Vec3 *normal, int num_lights, const PointLight *lights, const Vec4 *cam_pos, i32 *out_spec, LightColor *out_color, i32 prebaked_base_light);

/**
 * @brief Compute a higher-precision light value. Returns light level in Q2.6 fixed point (0..31 in integer part, 6 fractional bits giving 0..63 fractional range).
 * @param dist_q16
 * @param emit_level
 */
int light_from_dist_hires(i32 dist_q16, int emit_level);

#endif /* LIGHT_H */