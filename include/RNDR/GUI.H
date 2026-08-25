/* Copyright (c) 2026 Burak Yazar */

#ifndef GUI_H
#define GUI_H

#include "../CORE/TYPES/MATH.H"

/*
 * GUI - Shared immediate-mode UI primitives for Mode X (320x240x256).
 *
 * These are stateless drawing helpers used by both the editor (CEDIT)
 * and any in-game overlay (HUD, menus). They depend only on GFX/VGA.H.
 *
 * Convention: `dst` is always the VGA back-buffer from vga_backbuffer().
 */

/** @brief Fill a rectangle with a solid colour. */
void gui_fill_rect(u8 *dst, int x0, int y0, int x1, int y1, u8 color);

/** @brief Draw a 1-pixel-wide rectangle outline. */
void gui_draw_rect(u8 *dst, int x0, int y0, int x1, int y1, u8 color);

/** @brief Return non-zero if screen point (px,py) lies inside the rectangle. */
int gui_point_in_rect(int px, int py, int x0, int y0, int x1, int y1);

/** @brief  a framed panel: solid fill, outer border, inset inner border.
 *  @param col_bg    - fill colour
 *  @param col_outer - colour of the outer 1-pixel border at (x0,y0)-(x1,y1)
 *  @param col_inner - colour of the inner 1-pixel border at (x0+1,y0+1)-(x1-1,y1-1)
 */
void gui_panel(u8 *dst, int x0, int y0, int x1, int y1,
               u8 col_bg, u8 col_outer, u8 col_inner);

#endif /* GUI_H */
