// Copyright (c) 2026 Burak Yazar
// SPDX-License-Identifier: MIT
#ifndef VGA_H
#define VGA_H

#include "../CORE/TYPES/GFX.H"
#include "../RNDR/CAMER.H"

#define SCREEN_SIZE (SCREEN_H * SCREEN_W)
#define MODEX_PITCH_BYTES 80ul
#define MODEX_HEIGHT 240ul
#define MODEX_WIDTH 320ul
#define MODEX_PAGE_BYTES (MODEX_PITCH_BYTES * MODEX_HEIGHT)
#define MODEX_PAGE_WORDS (MODEX_PAGE_BYTES / 2)
#define MODEX_SCREEN_DWORDS 19200UL

/* Unified Linear Rasterizer Macros for DOS and WIN64 */
#ifndef PI
#define PI 3.14159265359f
#endif
#define RAST_PITCH SCREEN_W
#define RAST_ROW(d, x, y) ((d) + (x) + (y) * SCREEN_W)
#define RAST_ZBUF_STEP 1
#define RAST_X_STEP(s) (s)
#define RAST_X_REM(xr, x) ((xr) - (x) + 1)
#define RAST_BAYER_COL(x) (x)
#define RAST_DUOW4 duow_dx
#define RAST_DVOW4 dvow_dx
#define RAST_DOOW4 doow_dx
#define RAST_DLIT4 dlit_dx
#define RAST_DSPEC4 dspec_dx

#ifdef PLATFORM_WIN64
#define MAX_DRAW_DISTANCE FX_FROM_INT(16) /* Maximum world unit to render */
#else
#define MAX_DRAW_DISTANCE FX_FROM_INT(8) /* Maximum world unit to render */
#endif

#ifndef PLATFORM_WIN64
#define MAX_SUB_SPAN 4 /* Max chunks (4 pixels each) to draw in one sub-span. 4 chunks = 16 pixels. */
#else
#define MAX_SUB_SPAN 2 /* Max pixels to draw in one sub-span of affine interpolation (must be <= 255 for 8-bit fixed-point steps) */
#endif

#define FOG_DISTANCE_LEVEL 32

/* Fog target color ramp in 6-bit DAC values (0..63). */
#define FOG_NEAR_LVL 0
#define FOG_NEAR_R 50
#define FOG_NEAR_G 48
#define FOG_NEAR_B 54

#define FOG_MID_LVL 10
#define FOG_MID_R 40
#define FOG_MID_G 38
#define FOG_MID_B 44

#define FOG_FAR_LVL 22
#define FOG_FAR_R 24
#define FOG_FAR_G 22
#define FOG_FAR_B 27

#define FOG_VOID_LVL 31
#define FOG_VOID_R 12
#define FOG_VOID_G 11
#define FOG_VOID_B 14

extern u8 fog_shade_table[FOG_DISTANCE_LEVEL][256];

/**
 * @brief Build fog shade table from current VGA palette. Call once after loading palette.
 *        Blends each palette entry toward (FOG_R, FOG_G, FOG_B) across FOG_DISTANCE_LEVEL steps.
 * @param pal Pointer to 768-byte DAC palette (256*RGB, 6-bit values 0..63).
 */
void vga_init_fog_colormap(const u8 *pal);

/**
 * @brief Rebuild fog shade table with an explicit fog target color.
 * @param pal Pointer to 768-byte DAC palette (256*RGB, 6-bit values 0..63).
 * @param r Fog target red   (0..63)
 * @param g Fog target green (0..63)
 * @param b Fog target blue  (0..63)
 */
void vga_set_fog_color_dac(const u8 *pal, int r, int g, int b);

/**
 * @brief Set fog depth range. Precomputes reciprocal for per-pixel fog from oow values.
 *        Call once after world load or when fog parameters change.
 * @param z_start Near fog boundary (Q16.16). Closer than this = no fog.
 * @param z_end   Far fog boundary (Q16.16). Further than this = full fog.
 */
void vga_set_fog_range(i32 z_start, i32 z_end);

/**
 * @brief Getter for fog
 * @return fog_enabled
 */
int vga_get_fog_enabled(void);

/**
 * @brief Compute fog level for a given camera-space z distance.
 * @param z_cam Camera-space depth (Q16.16, must be > 0)
 * @return Fog level 0..31 (0 = no fog, 31 = full fog), or 0 if fog disabled.
 */
int vga_fog_level_from_z(i32 z_cam);

/**
 * @brief Get internal fog parameters for post-processing effects.
 * @param oow_near Output pointer for 1/z_start
 * @param oow_far Output pointer for 1/z_end
 * @param recip_range Output pointer for reciprocal range
 */
void vga_get_fog_params(i32 *oow_near, i32 *oow_far, i32 *recip_range);

/* Reciprocal LUT settings for perspective texture mapping.
 * Shift 5 means entries are spaced by 32 in Q16.16 (i.e. ~0.00048).
 * Size 32768 covers oow up to 16.0 (z down to 0.0625). */
#define RECIP_OOW_SHIFT 5
#define RECIP_OOW_SIZE 32768

/**
 * @brief Initialize the reciprocal-of-oow lookup table. Call once at startup.
 */
void vga_init_recip_oow(void);

/**
 * @brief Fast reciprocal lookup to convert oow back to z
 * @param oow Depth buffer value (1/z) in Q16.16
 * @return z in Q16.16
 */
i32 vga_recip_oow_lookup(i32 oow);

#define GRID_SIZE 16 /* Total number of grid lines */
#define GRID_SIZE_FX FX_FROM_INT(GRID_SIZE)
#define GRID_HALF 8
#define GRID_HALF_FX FX_FROM_INT(GRID_HALF)
#define GRID_STEP_FX FX_ONE
#define GRID_COLOR 255

extern u8 *vga_vram;      /* Pointer to VGA memory at 0xA0000 */
extern u32 *depth_buffer; /* Pointer to depth buffer */
extern int using_nearptr;
extern int using_dosmem;
extern int current_page;
extern bool g_lighting_pbr;

extern u8 bayer4x4[4][4];
extern const u8 g_clamp0_31[256];

typedef struct TriSetup_s
{
    const Vec2 *vt, *vm, *vb;
    i32 vt_uow, vm_uow, vb_uow;
    i32 vt_vow, vm_vow, vb_vow;
    i32 vt_oow, vm_oow, vb_oow;
    i32 vt_lit, vm_lit, vb_lit;
    i32 vt_spec, vm_spec, vb_spec;
    i32 dvm, dtb, dmb;
    i32 duow_tm, dvow_tm, doow_tm, dlit_tm, dspec_tm;
    i32 duow_tb, dvow_tb, doow_tb, dlit_tb, dspec_tb;
    i32 duow_mb, dvow_mb, doow_mb, dlit_mb, dspec_mb;
    i32 duow_dx, dvow_dx, doow_dx, dlit_dx, dspec_dx;
    int y_top, y_bot, y_mid;
    u32 tex_mask;
    int tex_shift;
    u8 dominant_color;
#ifdef PLATFORM_WIN64
    const V2PT *pbr_vt;
    float dwx_dx, dwy_dx, dwz_dx;
    float dnx_dx, dny_dx, dnz_dx;
    float dtx_dx, dty_dx, dtz_dx;
    float doow_f_dx, duow_f_dx, dvow_f_dx;
    float dwx_dy, dwy_dy, dwz_dy;
    float dnx_dy, dny_dy, dnz_dy;
    float dtx_dy, dty_dy, dtz_dy;
    float doow_f_dy, duow_f_dy, dvow_f_dy;
#endif
} TriSetup;

/* A_VGA.ASM functions */

/**
 * @brief Set VGA plane for a given x coordinate
 * @param x x coordinate in screen space
 */
extern void a_vga_set_plane_for_x(int x);

/**
 * @brief Set VGA to Abrash's Mode X configuration
 */
extern void a_vga_set_abrashx(void);

/**
 * @brief Wait for vertical retrace
 */
extern void a_vga_wait_retrace(void);

/**
 * @brief Clear VGA memory with a given value
 * @param dst pointer to start of VGA memory
 * @param value byte value to fill
 * @param nbytes number of bytes to clear
 */
extern void a_vga_clear_ram(void *dst, u8 value, u32 nbytes);

/**
 * @brief Put pixel at (x, y: integers) with given color
 * @param dst pointer to start of VGA memory
 * @param x x coordinate in screen space
 * @param y y coordinate in screen space
 * @param color color index to draw
 */
extern void a_vga_put_pixel(u8 *dst, i32 x, i32 y, u8 color);

/**
 * @brief Blit whole block of screen memory to vga memory
 * @param vram Pointer to the vga memory
 * @param backbuffer pointer to linear memory
 */
extern void a_vga_blit_ram_vram(u8 *vram, const u8 *backbuffer);

/**
 * @brief Blit a rectangle from system RAM to VGA memory
 * @param dst
 * @param src
 * @param x
 * @param y
 * @param w
 * @param h
 */
void a_vga_blit_ram(u8 *dst, const u8 *src, u16 x, u16 y, u16 w, u16 h);

/**
 * @brief Draw a solid vertical column
 * @param dst pointer to start of VGA memory
 * @param height height of column in pixels
 * @param color color index to fill
 */
extern void a_vga_draw_column_solid(u8 *dst, int height, u8 color);

/**
 * @brief Draw a textured vertical column
 * @param dst pointer to start of VGA memory
 * @param height height of column in pixels
 * @param tex pointer to texture data
 * @param v_fp starting V texture coordinate (16.16 fixed-point)
 * @param v_step V texture coordinate step per pixel (16.16 fixed-point)
 * @param u U texture coordinate (0-255)
 */
extern void a_vga_draw_column_tex(u8 *dst, int height, const u8 *tex, int v_fp, int v_step, u8 u);

/* VGA.C functions - ModeX included */

/**
 * @brief Flip the VGA backbuffer to the front buffer by fast blitting
 * @param backbuffer Pointer to the backbuffer
 */
void vga_flip(const u8 *backbuffer);

/**
 * @brief Get a pointer to the VGA backbuffer
 * @return pointer to the VGA backbuffer
 */
u8 *vga_backbuffer(void);

/**
 * @brief Wait for vertical retrace
 */
void vga_wait_retrace(void);

/**
 * @brief Set VGA plane for a given x coordinate
 * @param x x coordinate in screen space
 */
void vga_set_plane_for_x(int x);

/**
 * @brief Set VGA to Mode X (320x240x256)
 */
void vga_set_modeX(void);

/**
 * @brief Set VGA 256-color palette from a 768-byte DAC array
 * @param dac3x256 pointer to the DAC array
 */
void vga_set_palette256(const u8 *dac3x256);

/**
 * @brief Set VGA to text mode (80x25x16)
 */
void vga_set_text_mode(void);

/**
 * @brief Draw a line using Bresenham's algorithm
 * @param dst pointer to start of VGA memory
 * @param x0 starting x coordinate in screen space
 * @param y0 starting y coordinate in screen space
 * @param x1 ending x coordinate in screen space
 * @param y1 ending y coordinate in screen space
 * @param color color of the line
 */
void vga_draw_bresenham_line(u8 *dst, i32 x0, i32 y0, i32 x1, i32 y1, u8 color);

/**
 * @brief
 * @param dst
 * @param x0
 * @param y0
 * @param x1
 * @param y1
 * @param color
 */
void vga_draw_curved_line(u8 *dst, i32 x0, i32 y0, int height, u8 color);

/**
 * @brief Draw a 2D wireframe sphere (circle)
 * @param dst pointer to start of VGA memory
 * @param cx center x coordinate
 * @param cy center y coordinate
 * @param r radius
 * @param color color of the wireframe
 */
void vga_draw_wire_sphere(u8 *dst, int cx, int cy, int r, u8 color);

/**
 * @brief Draw a 2D wireframe cube (rectangle)
 * @param dst pointer to start of VGA memory
 * @param x0 left x coordinate
 * @param y0 top y coordinate
 * @param x1 right x coordinate
 * @param y1 bottom y coordinate
 * @param color color of the wireframe
 */
void vga_draw_wire_cube(u8 *dst, int x0, int y0, int x1, int y1, u8 color);

/**
 * @brief Draw a 2D wireframe capsule (stadium)
 * @param dst pointer to start of VGA memory
 * @param cx1 first center x coordinate
 * @param cy1 first center y coordinate
 * @param cx2 second center x coordinate
 * @param cy2 second center y coordinate
 * @param r radius
 * @param color color of the wireframe
 */
void vga_draw_wire_capsule(u8 *dst, int cx1, int cy1, int cx2, int cy2, int r, u8 color);

/**
 * @brief Draw a single grass blade with phase offset
 * @param dst pointer to start of VGA memory
 * @param x0 x start
 * @param y0 y start
 * @param height height of the blade
 * @param oow 1/z value for depth test
 * @param color color of the blade
 * @param phase phase offset for wind
 * @param wind_x screen-space wind sway
 */
void vga_draw_grass_blade(u8 *dst, const V2PT *v, int height, u8 color, i32 phase, i32 wind_x, const ClipRect *clip_rect);

/**
 * @brief Draw a grid based on the camera's view
 * @param dst pointer to start of VGA memory
 * @param cam pointer to the camera object
 */
void vga_draw_grid(u8 *dst, const Camera *cam, const ClipRect *clip_rect, bool enabled);

/**
 * @brief Draw a solid horizontal scanline
 * @param dst pointer to start of VGA memory
 * @param y y coordinate of scanline
 * @param x0 starting x coordinate of scanline
 * @param x1 ending x coordinate of scanline
 * @param col solid color to fill
 */
void vga_draw_scanline_solid(u8 *dst, int y, int x0, int x1, const u8 col);

/**
 * @brief Draw an affine textured horizontal scanline with plane batching
 * @param dst pointer to start of VGA memory
 * @param y y coordinate of scanline
 * @param x0 starting x coordinate of scanline
 * @param x1 ending x coordinate of scanline
 * @param tex pointer to texture data
 * @param u_fp starting U texture coordinate (16.16 fixed-point)
 * @param v_fp starting V texture coordinate (16.16 fixed-point)
 * @param du_fp U texture coordinate step per pixel (16.16 fixed-point)
 * @param dv_fp V texture coordinate step per pixel (16.16 fixed-point)
 */
void vga_draw_scanline_atex_pl(u8 *dst, int y,
                               int x0, int x1,
                               const u8 *tex,
                               i32 u_fp, i32 v_fp,
                               i32 du_fp, i32 dv_fp);

/**
 * @brief Draw a perspective-corrected textured horizontal span
 * @param surf pointer to the destination surface
 * @param y y coordinate of scanline
 * @param x0 starting x coordinate of scanline
 * @param x1 ending x coordinate of scanline
 * @param tex pointer to texture data
 * @param uow0 starting U/Z value at the start of scanline
 * @param vow0 starting V/Z value at the start of scanline
 * @param oow0 starting 1/Z value at the start of scanline
 * @param duow_dx change in U/Z per pixel
 * @param dvow_dx change in V/Z per pixel
 * @param doow_dx change in 1/Z per pixel
 * @param clip pointer to the clipping rectangle
 */
void vga_draw_span_persp_pl(Surface8 *surf, int y, int x0, int x1, const u8 *tex, i32 uow0, i32 vow0, i32 oow0, i32 duow_dx, i32 dvow_dx, i32 doow_dx, const ClipRect *clip);

/**
 * @brief Clear a VGA page with a given value
 * @param page page number (0 or 1)
 * @param value byte value to fill
 * @param nbytes number of bytes to clear
 */
void vga_clear_page(int page, u8 value, u32 nbytes);

/**
 * @brief Clear the depth buffer (call once per frame before rendering)
 */
void vga_clear_depth(void);

/**
 * @brief Draws a wireframe triangle using given screen space vectors.
 * @param dst pointer to start of VGA memory
 * @param v1 pointer to the first vertex (x, y in screen space)
 * @param v2 pointer to the second vertex (x, y in screen space)
 * @param v3 pointer to the third vertex (x, y in screen space)
 * @param color color index for edges
 */
void vga_draw_triangle_wire(u8 *dst, const Vec2 *v1, const Vec2 *v2, const Vec2 *v3, u8 color);

/**
 * @brief Draw a solid filled triangle in screen space
 * @param dst pointer to start of VGA memory
 * @param v1 pointer to the first vertex (x, y in screen space)
 * @param v2 pointer to the second vertex (x, y in screen space)
 * @param v3 pointer to the third vertex (x, y in screen space)
 * @param clip pointer to the clipping rectangle
 * @param color color index to fill the triangle
 */
void vga_draw_triangle_solid(u8 *dst, const Vec2 *v1, const Vec2 *v2, const Vec2 *v3, const ClipRect *clip, u8 color);

/**
 * @brief Draw a textured filled triangle using column-based rasterization
 * @param dst pointer to start of VGA memory
 * @param x0 x coordinate of the first vertex
 * @param y0 y coordinate of the first vertex
 * @param u0 U texture coordinate of the first vertex (16.16 fixed-point)
 * @param v0 V texture coordinate of the first vertex (16.16 fixed-point)
 * @param x1 x coordinate of the second vertex
 * @param y1 y coordinate of the second vertex
 * @param u1 U texture coordinate of the second vertex (16.16 fixed-point)
 * @param v1 V texture coordinate of the second vertex (16.16 fixed-point)
 * @param x2 x coordinate of the third vertex
 * @param y2 y coordinate of the third vertex
 * @param u2 U texture coordinate of the third vertex (16.16 fixed-point)
 * @param v2 V texture coordinate of the third vertex (16.16 fixed-point)
 * @param tex pointer to texture data (assumed 64x64)
 * @param pminx clipping minimum x bound
 * @param pminy clipping minimum y bound
 * @param pmaxx clipping maximum x bound
 * @param pmaxy clipping maximum y bound
 */
void vga_draw_col_triangle_tex(u8 *dst, i32 x0, i32 y0, i32 u0, i32 v0, i32 x1, i32 y1, i32 u1, i32 v1, i32 x2, i32 y2, i32 u2, i32 v2, const u8 *tex, i32 pminx, i32 pminy, i32 pmaxx, i32 pmaxy);

/**
 * @brief Draw a affine textured filled triangle using edge-walking scanline rasterization
 * @param dst pointer to start of VGA memory
 * @param v1 pointer to the first vertex (x, y in 16.16 fixed-point)
 * @param v2 pointer to the second vertex (x, y in 16.16 fixed-point)
 * @param v3 pointer to the third vertex (x, y in 16.16 fixed-point)
 * @param tex pointer to texture data (assumed 64x64)
 * @param clip pointer to the clipping rectangle
 */
void vga_draw_scan_triangle_atex(u8 *dst, const V2PT *v1, const V2PT *v2, const V2PT *v3, const u8 *tex, const ClipRect *clip);

/**
 * @brief Draw a textured filled triangle using edge-walking scanline rasterization
 * @param dst pointer to start of VGA memory
 * @param v1 pointer to the first vertex (x, y in 16.16 fixed-point)
 * @param v2 pointer to the second vertex (x, y in 16.16 fixed-point)
 * @param v3 pointer to the third vertex (x, y in 16.16 fixed-point)
 * @param uv1 pointer to the first vertex's texture coordinates (u, v in 16.16 fixed-point)
 * @param uv2 pointer to the second vertex's texture coordinates (u, v in 16.16 fixed-point)
 * @param uv3 pointer to the third vertex's texture coordinates (u, v in 16.16 fixed-point)
 * @param tex pointer to texture data
 * @param tex_w texture width in pixels (must be power-of-2, e.g. 64 or 128)
 * @param clip pointer to Clipping rectangle struct
 */
void vga_draw_scan_triangle_tex(u8 *dst, const V2PT *v1, const V2PT *v2, const V2PT *v3, const u8 *tex, u32 tex_w, int tex_shift, const ClipRect *clip, int color_key);
void vga_draw_scan_triangle_tex_spans(u8 *dst, const V2PT *v1, const V2PT *v2, const V2PT *v3, const u8 *tex, const u8 *tex_spec, u32 tex_w, int tex_shift, const ClipSpans *spans, int color_key, int alpha);

/**
 * @brief Terrain blend rasterizer
 * @param dst pointer to start of VGA memory
 * @param v1
 * @param v2
 * @param v3
 * @param tex_ptrs
 * @param tex_masks
 * @param tex_shifts
 * @param blend_map
 * @param blend_w
 * @param spans
 */
void vga_draw_scan_triangle_terr_spans(u8 *dst, const V2PT *v1, const V2PT *v2, const V2PT *v3,
                                       const u8 **tex_ptrs, const u8 *tex_spec, const u32 *tex_masks, const u32 *tex_shifts,
                                       const u8 *blend_map, u32 blend_w, u32 blend_h, const ClipSpans *spans, int alpha);

#ifdef PLATFORM_WIN64
/**
 * @brief PBR variant of the triangle rasterizer for Win64.
 * Assumes dst is a 32-bit truecolor buffer. Fetches from multiple material
 * maps and evaluates the Cook-Torrance BRDF per pixel.
 */
void vga_draw_scan_triangle_pbr_spans(u8 *dst, const V2PT *v1, const V2PT *v2, const V2PT *v3,
                                      const u8 *tex_albedo, const u8 *tex_normal, const u8 *tex_rough, const u8 *tex_metal,
                                      u32 tex_w, int tex_shift, const ClipSpans *spans, int color_key, int alpha);
#endif
/**
 * @brief Draw a backface-culled wireframe triangle.
 *
 * Takes world-space vertices, culls back-facing triangles, transforms to
 * camera space, near-plane clips each edge, projects to screen, and draws
 * three Cohen-Sutherland-clipped Bresenham lines.
 *
 * @param dst   VGA back-buffer pointer
 * @param v0    World-space vertex 0
 * @param v1    World-space vertex 1
 * @param v2    World-space vertex 2
 * @param cam   Active camera (provides view matrix, z_near, focal length)
 * @param color Palette index for the lines
 */
void vga_draw_wireframe_tri(u8 *dst,
                            const Vec4 *v0, const Vec4 *v1, const Vec4 *v2,
                            const Camera *cam, u8 color);

#ifdef PLATFORM_WIN64
extern int crt_on;
extern int aberr_on;
extern int smear_on;

void vga_set_shader_warp(float warp_x, float warp_y);
void vga_set_shader_mask(float mask_dark, float mask_light);
void vga_set_shader_aberration(float shift_x, float shift_y);
void vga_set_shader_scanline(float weight);
void vga_set_shader_smear(float smear);
#else
#define vga_set_shader_warp(x, y) ((void)0)
#define vga_set_shader_mask(d, l) ((void)0)
#define vga_set_shader_aberration(x, y) ((void)0)
#define vga_set_shader_scanline(w) ((void)0)
#define vga_set_shader_smear(s) ((void)0)
#endif

#endif /* VGA_H */