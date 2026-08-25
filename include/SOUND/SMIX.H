/* Copyright (c) 2026 Burak Yazar */

#ifndef SMIX_H
#define SMIX_H

/************************ Spatial Sound Mixer **********************************/
#include "../CORE/TYPES/AUDIO.H"
#include "../RNDR/CAMER.H"

/* -------------------------------------------------------------------------
   Global state: array of spatial sound sources
   ------------------------------------------------------------------------- */
typedef struct SpatialResult_s
{
    i32 vol; /* combined volume (Q16.16) */
    i32 pan; /* stereo pan: -FX_ONE = full left, +FX_ONE = full right */
} SpatialResult;

extern SoundSource g_snd_sources[MAX_SND_SRC];

/**
 * @brief Initialize the software mixer and start DMA streaming.
 * @param rate Sample rate in Hz (must match BlastConfig, e.g. 22050)
 * @return 1 on success, 0 on failure.
 */
int smix_init(u16 rate);

/**
 * @brief Shut down the mixer and stop DMA streaming.
 */
void smix_shutdown(void);

/**
 * @brief Per-frame update: compute spatial volumes, mix sources, play via DMA.
 *        Call this once per frame from the main loop.
 * @param cam Camera pointer (listener position + orientation)
 */
void smix_update(const Camera *cam);

/**
 * @brief Play a sound in a specific slot.
 * @param slot    Slot index (0..MAX_SND_SRC-1)
 * @param pcm     Loaded PCM file
 * @param pos     World-space position (Q16.16)
 * @param vol     Base volume (Q16.16, 0..FX_ONE)
 * @param min_d   Full-volume radius (Q16.16)
 * @param max_d   Max audible radius (Q16.16)
 * @param loop    true to loop, false for one-shot
 * @return 1 on success, 0 on invalid slot.
 */
int smix_play(int slot, PCM8UFile *pcm, const Vec4 *pos,
              i32 vol, i32 min_d, i32 max_d, bool loop);

/**
 * @brief Stop playback in a specific slot.
 * @param slot Slot index (0..MAX_SND_SRC-1)
 */
void smix_stop(int slot);

/**
 * @brief Stop all sound sources.
 */
void smix_stop_all(void);

/**
 * @brief Update position of a sound source (for moving emitters).
 * @param slot Slot index
 * @param pos  New world-space position
 */
void smix_set_position(int slot, const Vec4 *pos);

/**
 * @brief Update spatial parameters of a sound source.
 * @param slot  Slot index (0..MAX_SND_SRC-1)
 * @param vol   Base volume (Q16.16, 0..FX_ONE)
 * @param min_d Full-volume radius (Q16.16)
 * @param max_d Max audible radius (Q16.16)
 * @param loop  true to loop, false for one-shot
 */
void smix_set_params(int slot, i32 vol, i32 min_d, i32 max_d, bool loop);

/**
 * @brief Get the number of successful buffer fills (debug diagnostic).
 * @return Fill count since init.
 */
u32 smix_get_fill_count(void);

#endif /* SMIX_H */
