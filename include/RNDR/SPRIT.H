/* Copyright (c) 2026 Burak Yazar */

#ifndef SPRIT_H
#define SPRIT_H

#include "../CORE/TYPES/GFX.H"
#include "../CORE/TYPES/WORLD.H"
#include "../CORE/TYPES/CAM.H"

/**
 * @brief Frees sprite object
 * @param s Pointer to Sprite object
 */
void sprite_free(Sprite *s);

/**
 * @brief Loads Raw image to Sprite object
 * @param s Pointer to sprite object (out)
 * @param raw Path to raw image file
 * @param w Width of image
 * @param h Height of image
 * @return 1 on load success 0 on fail
 */
int load_sprite(Sprite *s, const u8 *raw, u32 w, u32 h);

/**
 * @brief Draws Sprite at given position in screen coordinates
 * @param buf VRAM base pointer
 * @param x X screren coordinate of sprite
 * @param y Y screren coordinate of sprite
 * @param s Pointer to sprite object
 */
void draw_sprite(u8 *buf, i32 x, i32 y, const Sprite *s);

/**
 * @brief Draw sprite scaled to arbitrary size with color-key transparency.
 * @param buf  VRAM base pointer
 * @param cx   Screen X center of sprite
 * @param cy   Screen Y center of sprite
 * @param dw   Destination width in pixels
 * @param dh   Destination height in pixels
 * @param s    Pointer to sprite object
 * @param color_key Palette index to treat as transparent (-1 = opaque)
 * @param clip Clipping rectangle
 */
void draw_sprite_scaled(u8 *buf, int cx, int cy, int dw, int dh,
                        const Sprite *s, int color_key,
                        int light_level, int fog_level,
                        const ClipRect *clip);

/**
 * @brief               Draw a billboard sprite in 3D: always faces camera.
 * @param buf           VRAM base pointer
 * @param world_pos     World-space position of sprite center (Q16.16)
 * @param world_w       World-space width of sprite (Q16.16)
 * @param world_h       World-space height of sprite (Q16.16)
 * @param cam           Camera
 * @param s             Sprite to draw
 * @param color_key     Palette index to treat as transparent (-1 = opaque)
 * @param num_lights    Number of world lights
 * @param lights        Pointer to world light array
 * @param clip          Clipping rectangle
 */
void draw_sprite_billboard(u8 *buf, const Vec4 *world_pos,
                           i32 world_w, i32 world_h,
                           const Camera *cam, const Sprite *s,
                           int color_key,
                           int num_lights, const PointLight *lights,
                           const ClipRect *clip);

#endif