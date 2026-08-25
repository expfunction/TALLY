/* Copyright (c) 2026 Burak Yazar */

#ifndef AUDIO_TYPES_H
#define AUDIO_TYPES_H

#include "MATH.H"

/* PCM file header (8-bit unsigned WAV) */
typedef struct PCM8UFileHeader_s
{
    u8 riff_id[4];
    u32 riff_size;
    u8 wave_id[4];
    u8 fmt_id[4];
    u32 fmt_size;
    u16 audio_format;
    u16 num_channels;
    u32 sample_rate;
    u32 byte_rate;
    u16 block_align;
    u16 bits_per_sample;
    u8 data_id[4];
    u32 data_size;
} PCM8UFileHeader;

/* Loaded 8-bit unsigned PCM file */
typedef struct PCM8UFile_s
{
    PCM8UFileHeader header;
    u8 *data;
    u32 length;
} PCM8UFile;

/* Sound Blaster hardware state */
typedef struct BlastState_s
{
    int present;
    u16 base;
    u8 irq, dma8;
    u16 sample_rate;
    volatile int playing;
    int irq_installed;
    u8 dsp_ver_major, dsp_ver_minor;
    u8 stereo;
} BlastState;

/* Sound Blaster card configuration */
typedef struct BlastConfig_s
{
    u16 base_port;
    u8 irq, dma8;
    u16 sample_rate;
} BlastConfig;

#define MAX_SND_SRC 8
#ifdef PLATFORM_WIN64
#define STREAM_HALF 512u
#else
#define STREAM_HALF 2048u
#endif
#define STREAM_SIZE (STREAM_HALF * 2)

/* Spatial sound source (runtime) */
typedef struct SoundSource_s
{
    PCM8UFile *pcm;
    Vec4 position;          /* world-space position Q16.16 */
    u32 cursor;             /* PCM sample offset (integer) */
    i32 base_vol;           /* volume at min_dist Q16.16 (0..FX_ONE) */
    i32 vol_fx;             /* current computed volume Q16.16 */
    i32 min_dist, max_dist; /* attenuation range Q16.16 world units */
    bool looping, active;
} SoundSource;

#endif /* AUDIO_TYPES_H */
