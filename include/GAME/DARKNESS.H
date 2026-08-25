/* Copyright (c) 2026 Burak Yazar */

#ifndef DARKNESS_H
#define DARKNESS_H

#include "../CORE/TYPES/GFX.H"
#include "../CORE/TYPES/MATH.H"

/**
 * @brief Initializes the Darkness subsystem. Loads the vignette sprite.
 * @return 1 on success, 0 on failure.
 */
int darkness_init(void);

/**
 * @brief Updates the darkness state, shrinking the vignette over time.
 * @param dt Delta time for the frame (in fixed point or float).
 */
void darkness_update(void);

/**
 * @brief Pushes the darkness vignette back out to its maximum radius.
 */
void darkness_recenter(void);

/**
 * @brief Renders the darkness vignette overlay.
 * @param dst VGA back-buffer pointer
 */
void darkness_render(u8 *dst);

/**
 * @brief Shuts down the Darkness subsystem, freeing resources.
 */
void darkness_shutdown(void);

#endif /* DARKNESS_H */
