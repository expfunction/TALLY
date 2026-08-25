/* Copyright (c) 2026 Burak Yazar */

#ifndef SKY_H
#define SKY_H

#include "../CORE/TYPES/GFX.H"
#include "CAMER.H"

/* --- Phase palette indices (tune to match ASSTS/PAL/HRMNAT.PAL) --- */

/* Zenith (top) and horizon (bottom) palette indices for each phase.
 * All values must stay within the blue ramp (HRMNAT.PAL indices 65-96)
 * to avoid crossing the black ramp-reset at index 65 or 97.
 * Blue ramp reference:  66=#03080D  70=#142741  80=#477FC2  93=#CAEFFF  95=#E3FBFF
 *
 * For orange/warm dusk, add a dedicated warm palette ramp and update these. */
#define SKY_NOON_TOP 74      /* dark blue zenith  #284974 */
#define SKY_NOON_HORIZON 92  /* pale blue horizon #B9E7FF */
#define SKY_NIGHT_TOP 64     /* near-black zenith #03080D */
#define SKY_NIGHT_HORIZON 65 /* dark navy horizon #142741 */
/* Dusk/dawn: lighter blues (bleached sky). Replace with warm-ramp values once available. */
#define SKY_DUSK_TOP 65     /* bright sky blue   #75BDFD */
#define SKY_DUSK_HORIZON 70 /* pale cyan         #DAF7FF */
#define SKY_DAWN_TOP 87
#define SKY_DAWN_HORIZON 94

/* Directional horizon tints: blended toward when camera faces the body.
 * Sun  → near-white glow (brightest entry in blue ramp).
 * Moon → deep dark navy (looking into the night side). */
#define SKY_SUN_GLOW_HORIZON 95  /* near-white  #E3FBFF */
#define SKY_MOON_DARK_HORIZON 68 /* deep navy   ~#060F1A */

/* Fogged-sky palette anchors (HRMNAT.PAL blue ramp 65..96).
 * Day keeps haze readable; night is intentionally much darker. */
#define SKY_FOG_DAY_TOP 235
#define SKY_FOG_DAY_HORIZON 235
#define SKY_FOG_NIGHT_TOP 235
#define SKY_FOG_NIGHT_HORIZON 235

extern Vec2 g_sky_wind; /* World-space wind vector (X, Z) in Q16.16 */

/* World-space diameter of sun/moon sprites (world units).
 * scr_px = SKY_BODY_SIZE * (f / SKY_BODY_DIST) = 20 * (160/200) = 16 px at default dist. */
#define SKY_BODY_SIZE FX_FROM_INT(20)

/* Cycle speed: radians/second. Full 2pi cycle ~33 minutes.*/
/*#define SKY_TOD_RATE FX_FROM_FLOAT(0.05f)*/
/* Cycle speed: radians/second. Full 2pi cycle 24 hours (real-time).*/
#define SKY_TOD_RATE FX_FROM_FLOAT(0.0000727f)

/* 2*pi in Q16.16 */
#define SKY_TWO_PI FX_FROM_FLOAT(6.28318f)

/* Number of background stars */
#define SKY_STAR_COUNT 48

/* Palette index for star dots */
#define SKY_STAR_COLOR 123

/* --- Time-of-day clock --------------------------------------------------- */

/* Game seconds in one full day (24 * 60 * 60). */
#define SKY_SECS_PER_DAY 86400

/* Offset of noon from midnight in game seconds. tod=0 maps to 12:00:00. */
#define SKY_NOON_SECS 43200

/* pixels per radian at 90-deg horizontal FOV across 320 pixels */
#define SKY_PX_PER_RAD FX_FROM_FLOAT(203.718f)

/* world-space distance for sun/moon billboard placement */
#define SKY_BODY_DIST FX_FROM_INT(200)

/* pi in Q16.16 */
#define SKY_PI FX_FROM_FLOAT(3.14159f)

/* Q16.16 representation of 2*pi (for star azimuth modulo) */
#define SKY_TWO_PI_INT 411775u

/* Time conversion: s_time_of_day (Q16.16 raw int, 0..411775) <-> game seconds (0..86399).
 *
 * Exact relationship: secs_since_noon = tod_raw * 86400 / 411775
 * The intermediate product (411775 * 86400 = 35.6B) overflows i32, so reduce by GCD=25:
 *   86400 / 25 = 3456   (SKY_SECS_NUM)
 *   411775 / 25 = 16471 (SKY_SECS_DEN)
 * Max intermediate: 411775 * 3456 = 1,423,094,400 < 2^31. Safe in i32. */
#define SKY_SECS_NUM 3456
#define SKY_SECS_DEN 16471

/* H:M:S representation of the current game time.
 * Derived entirely from s_time_of_day — no separate accumulator. */
typedef struct SkyTime_s
{
    int h; /* 0..23 */
    int m; /* 0..59 */
    int s; /* 0..59 */
} SkyTime;

/* Maximum number of background sprite layers */
#define MAX_SKY_LAYERS 4

typedef struct SkyLayer_s
{
    const Sprite *spr;  /* 2D texture (e.g., mountain range) */
    i32 base_elevation; /* Base vertical offset on screen (Q16.16) */
    int yaw_tiles;      /* Number of times the texture tiles in 360 degrees (parallax is implicitly yaw_tiles * spr->w / 1280) */
    i32 parallax_y;     /* Multiplier for camera pitch (Q16.16) */
    int color_key;      /* Palette index to treat as transparent (e.g., 0) */
    int is_active;      /* Toggle visibility */
} SkyLayer;

/**
 * @brief Initialise sky module with sun and moon sprite pointers.
 *        Seeds the star table. Call once after engine init.
 */
void sky_init(const Sprite *sun_spr, const Sprite *moon_spr);

/**
 * @brief Advance the time-of-day angle by dt (Q16.16 seconds).
 *        Updates sky gradient colors, star visibility, and ambient dark floor.
 *        Call once per frame from engine_update().
 */
void sky_time_of_day(i32 dt);

/* --- Engine-internal time API (external code must use engine_get/set_time) --- */

/**
 * @brief Convert the current time-of-day angle to a game H:M:S clock.
 *        tod=0 → 12:00:00 (noon), tod=π → 00:00:00 (midnight).
 *        One full day = SKY_SECS_PER_DAY game seconds = 2π radians.
 */
SkyTime sky_get_time(void);

/**
 * @brief Set the time-of-day angle from a game H:M:S value.
 *        Immediately updates s_time_of_day so the next frame reflects the new time.
 * @param h  Hour   (0..23)
 * @param m  Minute (0..59)
 * @param s  Second (0..59)
 */
void sky_set_time(int h, int m, int s);

/**
 * @brief Return the current advance speed (Q16.16 radians per real second).
 *        Default = SKY_TOD_RATE.
 */
i32 sky_get_tod_rate(void);
/** Return the raw Q16.16 radian angle (0..2π). */
i32 sky_get_tod_angle(void);
/** Set the raw Q16.16 radian angle directly. Clamped to [0, 2π). */
void sky_set_tod_angle(i32 angle);

/**
 * @brief Set the advance speed (Q16.16 radians per real second).
 *        Pass 0 to freeze time. Negative values are clamped to 0.
 */
void sky_set_tod_rate(i32 rate);
void sky_set_tod_paused(int paused);
int sky_is_tod_paused(void);
void sky_set_enabled(int enabled);
int sky_is_enabled(void);

/**
 * @brief Draw a top-to-horizon gradient sky within the portal clip spans.
 * @param dst   VGA back-buffer pointer
 * @param cam   Camera used for directional horizon tint
 * @param spans Per-scanline clip bounds of the sky portal
 */
void sky_draw(u8 *dst, const Camera *cam, const ClipSpans *spans);

/**
 * @brief Draw star dots within the portal clip spans. No-op when the sun is high (daytime).
 */
void sky_draw_stars(u8 *dst, const Camera *cam, const ClipSpans *spans);

/**
 * @brief Draw sun and moon as billboards within the portal clip spans.
 *        Sun is visible above horizon; moon when sun is below.
 */
void sky_draw_sun_moon(u8 *dst, const Camera *cam, const ClipSpans *spans);

/**
 * @brief Initialize a sky layer.
 */
extern int g_sky_xtoangle[320];

void sky_layers_init(void);
void sky_layer_init(int index, const Sprite *spr, int yaw_tiles, i32 parallax_y, i32 base_elevation, int color_key);

/**
 * @brief Draw horizontally tiling background layers within the portal clip spans.
 */
void sky_draw_layers(u8 *dst, const Camera *cam, const ClipSpans *spans);

#endif /* SKY_H */
