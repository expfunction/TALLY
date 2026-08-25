/* Copyright (c) 2026 Burak Yazar */

/* Player controller for CyberVGA — third-person adventure RPG */
#ifndef FPPLYER_H
#define FPPLYER_H

#include "../CORE/TYPES/MATH.H"
#include "../CORE/FIXED.H"
#include "ENTITY.H"

/* Movement constants */
#define FP_PLAYER_MOVE_SPEED FX_FROM_FLOAT(0.35f)  /* world units / second */
#define FP_PLAYER_TURN_SPEED FX_FROM_FLOAT(1.094f) /* ~120 deg/sec in radians */

/* Stat caps */
#define FP_PLAYER_MAX_HEALTH FX_ONE
#define FP_PLAYER_MAX_STAMINA FX_ONE

/* Animation clip indices (must match loaded mesh CAN file order) */
#define FP_PLAYER_ANIM_IDLE 0
#define FP_PLAYER_ANIM_WALK 1
#define FP_PLAYER_ANIM_RUN 2
#define FP_PLAYER_ANIM_TRANS_DUR FX_FROM_FLOAT(0.2f) /* idle<->walk blend duration */

typedef struct FpPlayer_s
{
    int ent_idx;          /* entity pool index (integer, -1 = not spawned) */
    i32 yaw;              /* world-space facing direction Q16.16 radians */
    i32 pitch;            /* world-space facing tilt direction Q16.16 */
    i32 health;           /* 0..PLAYER_MAX_HEALTH Q16.16 */
    i32 stamina;          /* 0..PLAYER_MAX_STAMINA Q16.16 */
    bool controls_active; /* whether can be controlled or not */
} FpPlayer;

extern FpPlayer g_fpplayer;

/**
 * @brief Initialise the first-person player from an already-spawned entity index.
 *        Registers the think callback that drives input each frame.
 * @param ent_idx Active entity pool index with type ENT_PLAYER
 */
void fp_player_init(int ent_idx);

#endif /* FPPLYER_H */
