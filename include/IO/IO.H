/* Copyright (c) 2026 Burak Yazar */

#ifndef IO_H
#define IO_H

#include "../CORE/TYPES/MATH.H"
#include "../CORE/FIXED.H"

/* *************************** Key Scan Codes ************************************* */

#define KEY_NONE 0x00
#define KEY_ENTER 0x1C
#define KEY_SPACE 0x39
#define KEY_BACKSPACE 0x0E
#define KEY_TAB 0x0F
#define KEY_SHIFT 0x2A
#define KEY_CTRL 0x1D
#define KEY_ALT 0x38
#define KEY_F1 0x3B
#define KEY_F2 0x3C
#define KEY_F3 0x3D
#define KEY_F4 0x3E
#define KEY_F5 0x3F
#define KEY_F6 0x40
#define KEY_F7 0x41
#define KEY_F8 0x42
#define KEY_F9 0x43
#define KEY_F10 0x44
#define KEY_F11 0x57
#define KEY_F12 0x58
#define KEY_LEFT 0x4B
#define KEY_RIGHT 0x4D
#define KEY_UP 0x48
#define KEY_DOWN 0x50
#define KEY_ESC 0x01
#define KEY_A 0x1E
#define KEY_B 0x30
#define KEY_C 0x2E
#define KEY_D 0x20
#define KEY_E 0x12
#define KEY_F 0x21
#define KEY_G 0x22
#define KEY_H 0x23
#define KEY_I 0x17
#define KEY_J 0x24
#define KEY_K 0x25
#define KEY_L 0x26
#define KEY_M 0x32
#define KEY_N 0x31
#define KEY_O 0x18
#define KEY_P 0x19
#define KEY_Q 0x10
#define KEY_R 0x13
#define KEY_S 0x1F
#define KEY_T 0x14
#define KEY_U 0x16
#define KEY_V 0x2F
#define KEY_W 0x11
#define KEY_X 0x2D
#define KEY_Y 0x15
#define KEY_Z 0x2C
#define KEY_0 0x0B
#define KEY_1 0x02
#define KEY_2 0x03
#define KEY_3 0x04
#define KEY_4 0x05
#define KEY_5 0x06
#define KEY_6 0x07
#define KEY_7 0x08
#define KEY_8 0x09
#define KEY_9 0x0A
#define KEY_MINUS 0x0C
#define KEY_EQUALS 0x0D
#define KEY_GRAVE 0x29
#define KEY_LBRACKET 0x1A
#define KEY_RBRACKET 0x1B
#define KEY_SEMICOLON 0x27

/* Maximum number of scancodes (0x00 - 0x7F) */
#define KEY_TABLE_SIZE 128

/* ************************** Keyboard API **************************************** */

/**
 * @brief Install hardware keyboard interrupt handler (INT 9h).
 *        Must be called before the main loop.
 *        Replaces kbhit()/getch() with direct port 0x60 scancode reading.
 */
void cv_io_keyboard_init(void);

/**
 * @brief Remove the custom keyboard handler and restore the original INT 9h.
 *        Must be called before exiting the program.
 */
void cv_io_keyboard_shutdown(void);

/**
 * @brief Returns non-zero if the given scancode key is currently held down.
 * @param scancode One of the KEY_* defines (0x00 - 0x7F)
 */
i32 cv_io_key_down(u8 scancode);

/**
 * @brief Returns non-zero if the given key was just pressed this frame
 *        (transition from up -> down since last cv_io_keyboard_poll()).
 * @param scancode One of the KEY_* defines
 */
i32 cv_io_key_pressed_now(u8 scancode);

/**
 * @brief Returns non-zero if the given key was just released this frame
 *        (transition from down -> up since last cv_io_keyboard_poll()).
 * @param scancode One of the KEY_* defines
 */
i32 cv_io_key_released(u8 scancode);

/**
 * @brief Snapshot the current key state into "previous" state.
 *        Call once per frame, BEFORE reading keys, so that
 *        cv_io_key_pressed_now / cv_io_key_released work correctly.
 */
void cv_io_keyboard_poll(void);

/* ---- Legacy single-key API (kept for backward compatibility) ---- */

/**
 * @brief Returns pressed key as scancode (legacy blocking call).
 *        Prefer the new non-blocking API for game loops.
 */
u8 cv_io_key_poll(void);

/* *********************************** Mouse API ********************************** */

/* Mouse IO */
typedef struct MouseIO_s
{
    i32 x, y;
    i32 dx, dy;
    i32 btn_left, btn_right, btn_middle;
    u8 buttons;
    i32 mouse_mode;
} MouseIO;

/* Mouse buttons */
#define CV_MOUSE_BUTTON_LEFT 0x01
#define CV_MOUSE_BUTTON_RIGHT 0x02
#define CV_MOUSE_BUTTON_MIDDLE 0x04
#define MOUSE_Y_SCALE FX_FROM_FLOAT(1.2f) /* 240/200 */

/* Mouse modes */
#define CV_MOUSE_MODE_CURSOR 0    /* Normal cursor mode (GUI, menus) */
#define CV_MOUSE_MODE_MOUSELOOK 1 /* FPS mouselook: locked to center */

/**
 * @brief Initialize mouse, return 0 on success
 * @param screen_w screen width in pixels
 * @param screen_h screen height in pixels
 * @param mode CV_MOUSE_MODE_CURSOR for 2D mouse, CV_MOUSE_MODE_MOUSELOOK for center snapped mouse
 */
i32 cv_io_mouse_init(i32 screen_w, i32 screen_h, i32 mode);

/**
 * @brief Set mouse operating mode
 * @param mode CV_MOUSE_MODE_CURSOR or CV_MOUSE_MODE_MOUSELOOK
 */
void cv_io_mouse_set_mode(i32 mode);

/**
 * @brief  mouse - updates internal mouse state
 */
void cv_io_mouse_poll(void);

/**
 * @brief Mouse state, should be called once per frame, non-zero if mouse is present.
 * @param out pointer to cv_mouse structure to fill
 */
i32 cv_io_mouse_state(MouseIO *out);

/** @brief Returns Non-zero if mouse button is down */
i32 cv_io_mouse_button_down(void);
#endif