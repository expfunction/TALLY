/* Copyright (c) 2026 Burak Yazar */

#ifndef PALCYC_H
#define PALCYC_H

#include "../CORE/TYPES/MATH.H"

/*
 * PALCYC - Palette Cycling and Screen Effect System
 *
 * Stores a working copy of the current VGA DAC palette and provides
 * routines to cycle, fade, and flush it to the hardware.
 *
 * Typical frame usage:
 *   palcyc_rotate(64, 32, -1);   // rotate a water ramp backward
 *   palcyc_flush_range(64, 32);  // upload only the changed range
 *
 * For a full-screen fade:
 *   palcyc_fade_step(2, 0, 0, 0);  // darken by 2 DAC steps toward black
 *   palcyc_flush_all();
 *
 * Call palcyc_init() once after load_palette_and_apply().
 * All palette values are 6-bit DAC (0..63).
 */

/* Working copy of the current DAC palette (256 * RGB, 6-bit values) */
extern u8 g_pal_dac[768];

/*
 * @brief Copy the given 768-byte DAC palette into g_pal_dac.
 *        Call once after load_palette_and_apply().
 */
void palcyc_init(const u8 *dac768);

/*
 * @brief Rotate `count` consecutive palette entries starting at `first`
 *        cyclically by `dir` positions in g_pal_dac (does NOT flush).
 * @param first  First palette index (0..255)
 * @param count  Number of entries in the range (must be >= 2)
 * @param dir    +1 = rotate forward (entry[0] <- entry[count-1])
 *               -1 = rotate backward (entry[count-1] <- entry[0])
 */
void palcyc_rotate(u8 first, u8 count, int dir);

/*
 * @brief Move all entries in g_pal_dac toward (tr, tg, tb) by `step` DAC units.
 *        Clamps each channel at the target. Does NOT flush.
 * @param step   DAC units to move per call (1..63)
 * @param tr     Target red   (0..63, 6-bit DAC)
 * @param tg     Target green (0..63, 6-bit DAC)
 * @param tb     Target blue  (0..63, 6-bit DAC)
 */
void palcyc_fade_step(int step, u8 tr, u8 tg, u8 tb);

/*
 * @brief Reset g_pal_dac to the original DAC palette stored at init time.
 *        Useful for restoring after a fade. Does NOT flush.
 */
void palcyc_restore(void);

/*
 * @brief Write `count` entries starting at `first` from g_pal_dac to the VGA DAC.
 *        Waits for vertical retrace before writing to avoid snow.
 */
void palcyc_flush_range(u8 first, u8 count);

/*
 * @brief Write all 256 entries from g_pal_dac to the VGA DAC.
 *        Waits for vertical retrace before writing to avoid snow.
 */
void palcyc_flush_all(void);

#endif /* PALCYC_H */
