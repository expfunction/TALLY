/* Copyright (c) 2026 Burak Yazar */

#ifndef TIME_H
#define TIME_H

/************************ Real-Time Tick System ********************************/
#include "TYPES/MATH.H"

#ifdef PLATFORM_WIN64
#define PLAT_NOW() plat_ticks()
#define PLAT_HZ() plat_ticks_per_sec()
#else
#define PLAT_NOW() ((i64)uclock())
#define PLAT_HZ() ((i64)UCLOCKS_PER_SEC)
#endif

/**
 * @brief Initialize the game clock. Call once before the main loop.
 * @param clk Pointer to GameClock struct
 */
void tick_init(GameClock *clk);

/**
 * @brief Mark the start of a new frame. Call at the top of each loop iteration.
 *        Updates dt, fps, and frame count.
 * @param clk Pointer to GameClock struct
 */
void tick_update(GameClock *clk);

#endif /* TIME_H */
