/* Copyright (c) 2026 Burak Yazar */

#ifndef CONSL_H
#define CONSL_H

#include "TYPES/MATH.H"

/* ****************** Console API ******************************* */

#define CONSOLE_MSG_LEN 256

/* Console window log buffer */
#define CONSL_MAX_LINES 24
#define CONSL_LINE_LEN 64
#define CONSL_FONT_SIZE 5
#define CONSL_LINE_HEIGHT 6

/* Console window position and size */
#define CONSL_X0 2
#define CONSL_Y0 2
#define CONSL_PAD 3

/* Colors */
#define CONSL_BG_COL 0
#define CONSL_FG_COL 255
#define CONSL_BD_COL 8

extern u8 ConsoleMsgBuf[CONSOLE_MSG_LEN];
extern u8 *ConsoleMsg;

void init_console(const char *init_msg, ...);
void append_console_msg(const u8 *msg, ...);
void clear_console_msg(void);

void show_console_window(void);
void hide_console_window(void);
int console_is_visible(void);

void console_log(const char *fmt, ...);
void draw_console_window(u8 *dst);

#endif /* CONSL_H */
