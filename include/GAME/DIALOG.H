/* Copyright (c) 2026 Burak Yazar */

#ifndef DIALOG_H
#define DIALOG_H

#include "../CORE/TYPES/MATH.H"

#define DIALOG_MAX_LINES 4 /* max lines displayed at once */
#define DIALOG_LINE_LEN 36 /* max chars per line (includes null) */

typedef struct Dialog_s
{
    char lines[DIALOG_MAX_LINES][DIALOG_LINE_LEN];
    int count; /* number of active lines */
} Dialog;

extern Dialog g_dialog;

/**
 * @brief Append a line of text to the dialogue box.
 *        Scrolls oldest line out when full.
 * @param text null-terminated string; truncated to DIALOG_LINE_LEN-1
 */
void dialog_push(const char *text);

/**
 * @brief Clear all dialogue lines.
 */
void dialog_clear(void);

/**
 * @brief Draw active lines into a screen rectangle using the 5x5 font.
 * @param dst  VGA back-buffer pointer
 * @param x    left edge of text area
 * @param y    top edge of text area
 * @param w    width of text area (unused, for future word-wrap)
 * @param h    height of text area (unused)
 */
void dialog_draw(u8 *dst, int x, int y, int w, int h);

#endif /* DIALOG_H */
