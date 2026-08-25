/* Copyright (c) 2026 Burak Yazar */

#ifndef ANIM_H
#define ANIM_H

/************************ Keyframe Animation System ****************************/
#include "../CORE/TYPES/WORLD.H"

#define MAX_ANIM_KEYS 16

typedef enum AnimMode_e
{
    ANIM_ONCE,
    ANIM_LOOP,
    ANIM_PINGPONG
} AnimMode;

typedef enum AnimState_e
{
    ANIM_STOPPED,
    ANIM_PLAYING,
    ANIM_PAUSED
} AnimState;

/** @brief Single keyframe: transform at a point in time */
typedef struct AnimKey_s
{
    i32 time;  /* Q16.16 seconds from clip start */
    Vec4 pos;  /* world-space position */
    Vec3 rot;  /* Euler angles (pitch, yaw, roll) Q16.16 radians */
    i32 scale; /* uniform scale Q16.16 (FX_ONE = 1.0) */
} AnimKey;

/** @brief Animation clip: sequence of keyframes */
typedef struct AnimClip_s
{
    const AnimKey *keys;
    u16 num_keys;
    i32 duration; /* = keys[num_keys-1].time, Q16.16 seconds */
} AnimClip;

/** @brief Per-object animation playback state */
typedef struct AnimPlayer_s
{
    const AnimClip *clip;
    AnimState state;
    AnimMode mode;
    i32 cursor;   /* current time Q16.16 seconds */
    i32 speed;    /* playback rate Q16.16 (FX_ONE = 1x) */
    i8 direction; /* +1 forward, -1 reverse (ping-pong) */
    u16 seg;      /* cached segment index */
} AnimPlayer;

/** @brief Initialize all animation players. Call once at startup. */
void anim_init(void);

/** @brief Assign a clip to a mesh slot and start playback. */
void anim_play(int mesh_idx, const AnimClip *clip, AnimMode mode, i32 speed);

/** @brief Stop animation. If reset=1, snap to first keyframe. */
void anim_stop(int mesh_idx, int reset);

/** @brief Pause playback (retains cursor position). */
void anim_pause(int mesh_idx);

/** @brief Resume paused playback. */
void anim_resume(int mesh_idx);

/**
 * @brief Advance all active animations by dt.
 *        Writes interpolated transforms into world->meshes[].
 *        Call once per frame before rendering.
 */
void anim_update(i32 dt, World *world);

/** @brief Check if an ANIM_ONCE animation has finished. */
int anim_is_done(int mesh_idx);

/** @brief Get the player for direct inspection. */
AnimPlayer *anim_get_player(int mesh_idx);

/* ---- Vertex animation (Quake-style per-vertex keyframes via .CAN) --------- */

/* Forward declaration — full definition in MESH/CAN.H */
struct CANFile_s;
struct CANClip_s;

/** @brief Per-object vertex animation playback state. */
typedef struct VAnimPlayer_s
{
    i8 clip_idx; /* index into CANFile.clips, -1 = stopped */
    AnimState state;
    AnimMode mode;
    i32 cursor;    /* current time Q16.16 seconds            */
    i32 speed;     /* playback rate Q16.16 (FX_ONE = 1x)     */
    i8 direction;  /* +1 forward, -1 reverse (ping-pong)     */
    u16 seg;       /* cached frame index within clip         */
    i32 trans_dur; /* Q16.16 total blend duration, 0 = none  */
    i32 trans_t;   /* Q16.16 elapsed blend time              */
} VAnimPlayer;

/** @brief Initialize all vertex animation players. Call once at startup. */
void vanim_init(void);

/** @brief Start vertex animation clip on a world mesh slot. */
void vanim_play(int mesh_idx, int clip_idx, AnimMode mode, i32 speed);

/**
 * @brief Start a new vertex animation clip with a linear cross-fade from the
 *        current pose. The outgoing pose is frozen at the moment of the call
 *        and lerped toward the incoming clip over trans_dur seconds.
 * @param trans_dur blend duration Q16.16 seconds (0 = immediate cut, same as vanim_play)
 */
void vanim_play_trans(int mesh_idx, int clip_idx, AnimMode mode, i32 speed,
                      i32 trans_dur, World *world);

/** @brief Stop vertex animation. If reset=1, resets scratch to rest pose. */
void vanim_stop(int mesh_idx, int reset);

/** @brief Pause vertex animation playback. */
void vanim_pause(int mesh_idx);

/** @brief Resume paused vertex animation. */
void vanim_resume(int mesh_idx);

/** @brief Check if an ANIM_ONCE vertex animation has finished. */
int vanim_is_done(int mesh_idx);

/** @brief Get the vertex animation player for direct inspection. */
VAnimPlayer *vanim_get_player(int mesh_idx);

/**
 * @brief Advance all active vertex animations by dt.
 *        Lerps vertex data into world->meshes[].mesh.cva_scratch[].
 *        Call once per frame before rendering.
 */
void vanim_update(i32 dt, World *world);

/**
 * @brief Fetch the interpolated local position of a slot (by name).
 *        Checks CAN slots first, falls back to CMS slots.
 * @param mesh_idx The world mesh index
 * @param name Slot name
 * @param out_pos Receives the Q16.16 local position
 * @param world Pointer to the world
 * @return 1 if found, 0 if not
 */
int vanim_get_slot_local_pos(int mesh_idx, const char *name, Vec4 *out_pos, World *world);

/** @brief Load or get a cached sound for an animation event. */
struct PCM8UFile_s *vanim_get_sound(const char *name);

#endif /* ANIM_H */
