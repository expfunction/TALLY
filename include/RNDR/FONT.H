/* Copyright (c) 2026 Burak Yazar */

#ifndef FONT_H
#define FONT_H

#include "../CORE/TYPES/MATH.H"

typedef enum CyberFont_e
{
    BASIC_4,
    BASIC_5,
    BASIC_8,
    JERSEY_10
} CyberFont;

/* Contains an 8x8 font map for unicode points U+0000 - U+007F (basic latin)*/
extern u8 font8x8_basic[128][8];

/* Contains a 4x4 font map for unicode points U+0000 - U+007F (basic latin)*/
extern u8 font4x4_basic[128][4];

/* Contains a 5x5 font map for unicode points U+0000 - U+007F (basic latin)*/
extern u8 font5x5_basic[128][5];

/* Contains a 10x10 font map for unicode points U+0000 - U+007F (Jersey 10)*/
extern u8 font10x10_jersey[128][10];

/**
 * @brief Draws one glyph directly on mode X buffer (no recursion, no side effects)
 * @param dst VRAM destination base pointer
 * @param x X coordinate of glyph
 * @param y Y coordinate of glyph
 * @param ch ANSI character of glyph
 * @param fg Foreground color
 * @param bg Background color
 */
void draw_glyph8x8X(u8 *dst, int x, int y, u8 ch, u8 fg, int bg);

/**
 * @brief Safe text draw: iterates once over the string; advances x deterministically
 * @param dst VRAM destination base pointer
 * @param x X coordinate of glyph
 * @param y Y coordinate of glyph
 * @param s String to draw
 * @param size Character size (4x4 || 8x8)
 * @param fg Foreground color
 * @param bg Background color
 */
void draw_text(u8 *dst, int x, int y, const char *s, CyberFont size, u8 fg, int bg);

/* fast glyph draw: does not set VGA registers, assumes valid coords */

/**
 * @brief Fast glyph draw: does not set VGA registers, assumes valid coords
 * @param dst VRAM destination base pointer
 * @param x X coordinate of glyph
 * @param y Y coordinate of glyph
 * @param ch ANSI character of glyph
 * @param fg Foreground color
 * @param bg Background color
 */
void draw_glyph8x8X_fast(u8 *dst, int x, int y, u8 ch, u8 fg, int bg);

/**
 * @brief Draws one 4x4 glyph directly on mode X buffer (no recursion, no side effects)
 * @param dst VRAM destination base pointer
 * @param x X coordinate of glyph
 * @param y Y coordinate of glyph
 * @param ch ANSI character of glyph
 * @param fg Foreground color
 * @param bg Background color
 */
void draw_glyph4x4X_fast(u8 *dst, int x, int y, u8 ch, u8 fg, int bg);
void draw_glyph5x5X_fast(u8 *dst, int x, int y, u8 ch, u8 fg, int bg);
void draw_glyph10x10X_fast(u8 *dst, int x, int y, u8 ch, u8 fg, int bg);
#endif /* FONT_H */