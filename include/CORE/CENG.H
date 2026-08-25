/* Copyright (c) 2026 Burak Yazar */

#ifndef CENG_H
#define CENG_H

/* Core */
#include "TYPES.H"
#include "CONSL.H"
#include "FIXED.H"
#include "MTEXTUR.H"
#include "MMATERL.H"
#include "TIME.H"

/* Subsystems */
#include "../IO/IO.H"
#include "../RNDR/RNDR.H"
#include "../MESH/CMS.H"
#include "../MESH/CAN.H"
#include "../GAME/ENTITY.H"
#include "../PHYS/PHYS.H"
#include "../SOUND/BLAST.H"
#include "../SOUND/SMIX.H"

/* Engine version */
#define CENG_VERSION_MAJOR 1
#define CENG_VERSION_MINOR 1
#define CENG_VERSION_PATCH 0
#define CENG_VERSION_STR "1.1.0"

/* World file magic. CWR8 introduces the material system: per-face material_id
 * arrays on brushes and single material_ids on billboards. CWR9 introduces
 * point light color and entity phys_height for capsule colliders.
 * CWR10 (saved as CWR1) introduces map triggers.
 * CWRA (version 11) introduces new particle properties. */
#define WORLD_MAGIC "CWRA"

/* Engine init flags */
#define ENG_F_ENT 0x01	   /* Initialize entity system */
#define ENG_F_ANIM 0x02	   /* Initialize animation system */
#define ENG_F_SND_OPT 0x04 /* Sound Blaster failure is non-fatal */

/* Engine configuration — fill before calling engine_init() */
typedef struct EngCfg_s
{
	const char *title;		/* pre-formatted console banner string */
	const char *pal_path;	/* palette file path */
	int mouse_mode;			/* CV_MOUSE_MODE_CURSOR or CV_MOUSE_MODE_MOUSELOOK */
	BlastConfig *blast_cfg; /* pointer to caller-owned BlastConfig */
	u8 flags;				/* ENG_F_* bitfield */
} EngCfg;

/* Globals owned by engine module */
extern World g_world;
extern GameClock g_clock;
extern int g_snd_ok;
extern i32 g_time_of_day;  /* sky time-of-day angle (Q16.16 radians, 0..2pi) */
extern i32 g_engine_time;  /* global engine time elapsed (Q16.16 seconds) */
extern int g_ambient_dark; /* world ambient darkness floor (0=bright, 22=night) */
extern Vec4 g_sun_dir;	   /* world-space unit direction toward sun (Q16.16); y<=0 = below horizon */
extern Vec2 sun_screen_pos;
extern Vec2 moon_screen_pos;

/* Dataset capture: set to 1 to force g_clock.dt = 0 this frame, freezing all
 * physics and animation so every G-buffer slice reflects the same world state. */
extern int g_capture_freeze;

int engine_init(const EngCfg *cfg);
void engine_update(Camera *cam);
void engine_render(Camera *cam, Surface8 *surf, const ClipRect *clip);
void engine_play_world_sounds(void);
void engine_shutdown(void);

/* --- Time-of-day control ------------------------------------------------- */

/** Return the raw sky angle in Q16.16 radians (0..2π). */
i32 engine_get_tod_angle(void);
/** Set the raw sky angle in Q16.16 radians. Clamped to [0, 2π). */
void engine_set_tod_angle(i32 angle);

/** Return the current game clock as H:M:S (derived from the sky angle). */
SkyTime engine_get_time(void);

/**
 * Jump the sky to the given game time.
 * @param h Hour (0..23), @param m Minute (0..59), @param s Second (0..59)
 */
void engine_set_time(int h, int m, int s);

/** Return the sky advance speed in Q16.16 radians per real second. */
i32 engine_get_tod_rate(void);

/**
 * Set the sky advance speed in Q16.16 radians per real second.
 * Pass 0 to freeze time. Use SKY_TOD_RATE for the default speed.
 */
void engine_set_tod_rate(i32 rate);

/** Pause (1) or resume (0) the sky time-of-day cycle. */
void engine_set_tod_paused(int paused);
/** Returns non-zero if the sky cycle is currently paused. */
int engine_is_tod_paused(void);
/** Enable (1) or completely disable (0) all sky rendering. */
void engine_set_sky_enabled(int enabled);
/** Returns non-zero if sky rendering is enabled. */
int engine_is_sky_enabled(void);

/**
 * @brief World file I/O
 * @param path
 */
int engine_load_world(const char *path);
int engine_save_world(const char *path);

/* Entity lifecycle helpers */

/**
 * @brief Spawn all WorldEntity templates from g_world into the entity pool.
 *        Loads meshes, sounds, lights, and physics components from each
 *        WorldEntity descriptor.  Call after engine_load_world().
 * @return Pool index of the player entity, or -1 if none was found.
 *         The caller is responsible for calling player_init() on that index.
 */
int engine_spawn_world_entities(void);

/**
 * @brief Kill and reset all active entities in the pool.
 *        Stops spatial sounds, removes particle emitters, frees cached
 *        entity meshes, and frees any PCM buffers loaded by
 *        engine_spawn_world_entities().  Safe to call when no entities
 *        are active (no-op).
 */
void engine_despawn_entities(void);

#endif
