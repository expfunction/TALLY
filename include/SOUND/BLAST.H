/* Copyright (c) 2026 Burak Yazar */

#ifndef BLAST_H
#define BLAST_H

/************************ Sound Blaster Implemention *****************************/
#include "../CORE/TYPES/AUDIO.H"

/* DSP port offsets from base */
#define SB_PORT_RESET 0x06
#define SB_PORT_READ 0x0A
#define SB_PORT_WRITE 0x0C
#define SB_PORT_READ_STAT 0x0E
#define SB_PORT_WRITE_BUFF 0x0C

/* DMA controller ports (master 8237, channels 0–3) */
#define DMA_STATUS_REG 0x08
#define DMA_MASK_REG 0x0A
#define DMA_MODE_REG 0x0B
#define DMA_CLEAR_FF 0x0C
#define DMA_BASE_CH0 0x00
#define DMA_COUNT_CH0 0x01
#define DMA_BASE_CH1 0x02
#define DMA_COUNT_CH1 0x03
#define DMA_BASE_CH2 0x04
#define DMA_COUNT_CH2 0x05
#define DMA_BASE_CH3 0x06
#define DMA_COUNT_CH3 0x07

/* DMA page ports for 8-bit channels */
#define DMA_PAGE_CH0 0x87
#define DMA_PAGE_CH1 0x83
#define DMA_PAGE_CH2 0x81
#define DMA_PAGE_CH3 0x82

/* PIC ports (for IRQ ack, if you later install a handler) */
#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC_EOI 0x20

/* SB and DMA maximum definitions. */
#define SB_MAX_BASES 4
#define DMA_BUF_MAX 65536u

/* -------------------------------------------------------------------------
   Global state
   ------------------------------------------------------------------------- */
extern BlastState g_blast;

/**
 * @brief Initialize and detect the card.
 * @param cfg Pointer to BlastConfig object
 * @return 1 on success, 0 on failure.
 */
int blast_init(const BlastConfig *cfg);

/**
 * @brief Shut down card usage (stops playback if still running).
 */
void blast_shutdown(void);

/**
 * @brief Load file into a DMA-able buffer and return pointer and length.
 * @param filename path to 8-bit unsigned PCM file
 * @param out_pcm pointer to buffer pointer to fill
 * @return 1 on success, 0 on failure.
 */
int blast_load_8u_pcm_file(const char *filename, PCM8UFile *out_pcm);

/**
 * @brief Simple, blocking playback of a *single* 8-bit unsigned mono PCM buffer via DMA.
 * @param data pointer to 8-bit unsigned PCM
 * @param length number of bytes
 * @param rate playback sample rate (Hz), overrides cfg->sample_rate for this call
 * @return 1 on success, 0 on failure.
 */
int blast_play_8u_block(const u8 *data, u32 length, u16 rate);

/**
 * @brief Non-blocking playback of a *single* 8-bit unsigned mono PCM buffer via DMA.
 * @param data pointer to 8-bit unsigned PCM
 * @param length number of bytes
 * @param rate playback sample rate (Hz), overrides cfg->sample_rate for this call
 * @return 1 on success, 0 on failure.
 */
int blast_play_pcm(const u8 *data, u32 length, u16 rate);

/**
 * @brief Poll each frame to chain DMA chunks for sounds larger than 64KB.
 */
void blast_update(void);

/**
 * @brief Check if sound is currently playing.
 * @return 1 if DMA playback is in progress, 0 if idle.
 */
int blast_is_playing(void);

/**
 * @brief Stop any ongoing playback immediately.
 */
void blast_stop(void);

/**
 * @brief Query function
 * @param void
 * @return 1 if SoundBlaster present, 0 if none found.
 */
int blast_is_present(void);

/**
 * @brief Check if stereo output is active (SB16 DSP >= 4.0).
 * @return 1 if stereo, 0 if mono.
 */
int blast_is_stereo(void);

/* -------------------------------------------------------------------------
   Streaming DMA API (timer-driven single-cycle)
   Used by the software mixer for continuous audio output.
   Each call to blast_stream_fill() programs a single-cycle DMA transfer
   of up to STREAM_HALF bytes. Timing is driven by ISR + uclock() fallback.
   ------------------------------------------------------------------------- */

/**
 * @brief Start streaming mode (sets sample rate, turns speaker on).
 * @param rate Sample rate in Hz (e.g. 22050)
 * @return 1 on success, 0 on failure.
 */
int blast_stream_start(u16 rate);

/**
 * @brief Stop streaming playback.
 */
void blast_stream_stop(void);

/**
 * @brief Check if the mixer should produce the next buffer.
 * @return 1 if a new buffer is needed, 0 if not.
 */
int blast_stream_needs_fill(void);

/**
 * @brief Send mixed audio to the Sound Blaster via single-cycle DMA.
 * @param data Pointer to mixed 8-bit unsigned PCM
 * @param len  Number of bytes (up to STREAM_HALF)
 */
void blast_stream_fill(const u8 *data, u32 len);
#endif