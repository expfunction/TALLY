/* Copyright (c) 2026 Burak Yazar */

#ifndef TXBOX_H
#define TXBOX_H

#include "../CORE/TYPES/MATH.H"

/*
 * TXBOX - Single-line text input widget for Mode X
 *
 * Usage (game loop):
 *   TxBox tb;
 *   txbox_init(&tb, 10, 100, 200, 32);
 *
 *   // each frame, after cv_io_keyboard_poll():
 *   int r = txbox_poll(&tb);
 *   if (r ==  1) { ... accept tb.buf ... txbox_clear(&tb); }
 *   if (r == -1) { ... cancel ... }
 *   txbox_draw(&tb, dst, COL_FG, COL_BG, COL_BORDER);
 *
 * Works in both MAIN and CEDIT — depends only on IO/IO.H, RNDR/FONT.H,
 * and GFX/VGA.H (all already included via PCH).
 */

#define TXBOX_MAX_LEN 63                 /* maximum characters in any TxBox */
#define TXBOX_FONT_SZ 8                  /* pixel size passed to draw_text */
#define TXBOX_HEIGHT (TXBOX_FONT_SZ + 4) /* total widget height in pixels */
#define TXBOX_BLINK_RATE 30              /* frames per cursor phase */

typedef struct TxBox_s
{
    char buf[TXBOX_MAX_LEN + 1]; /* null-terminated text buffer */
    int len;                     /* current character count */
    int max_len;                 /* capacity (clamped to TXBOX_MAX_LEN) */
    int x, y;                    /* top-left screen position in pixels */
    int w;                       /* widget width in pixels */
    int blink;                   /* frame counter for cursor blink */
} TxBox;

/*
 * @brief Initialize a TxBox. max_len is clamped to TXBOX_MAX_LEN.
 *        `w` is the total widget width in pixels.
 */
void txbox_init(TxBox *tb, int x, int y, int w, int max_len);

/*
 * @brief Empty the buffer without changing position or size.
 */
void txbox_clear(TxBox *tb);

/*
 * @brief Poll the keyboard and update the buffer.
 *        Call once per frame after cv_io_keyboard_poll().
 * @return  1  Enter pressed   — caller should read tb->buf and act
 *         -1  Esc pressed     — caller should cancel
 *          0  normal input    — no confirmation yet
 */
int txbox_poll(TxBox *tb);

/*
 * @brief Render the widget onto a Mode X back-buffer.
 * @param fg  Foreground palette index for text and cursor.
 * @param bg  Background palette index, or -1 for transparent (no fill).
 * @param bd  Border palette index, or -1 for no border.
 */
void txbox_draw(const TxBox *tb, u8 *dst, u8 fg, int bg, int bd);

#endif /* TXBOX_H */
