/* Copyright (c) 2026 Burak Yazar */

/* Player controller for CyberVGA — third-person adventure RPG */
#ifndef PLAYER_H
#define PLAYER_H

#include "../CORE/TYPES/MATH.H"
#include "../CORE/FIXED.H"
#include "ENTITY.H"

/* Movement constants */
#define PLAYER_MOVE_SPEED FX_FROM_FLOAT(0.35f)  /* world units / second */
#define PLAYER_TURN_SPEED FX_FROM_FLOAT(1.094f) /* ~120 deg/sec in radians */

/* Third-person camera arm */
#define PLAYER_CAM_ARM_LEN FX_FROM_FLOAT(0.6f)
#define PLAYER_CAM_ARM_HEIGHT FX_FROM_FLOAT(0.6f)
#define PLAYER_CAM_LAG FX_FROM_FLOAT(0.12f)
#define PLAYER_CAM_PITCH FX_FROM_FLOAT(0.174533f) /* ~-30 deg in radians */

/* Stat caps */
#define PLAYER_MAX_HEALTH FX_ONE
#define PLAYER_MAX_STAMINA FX_ONE

/* Animation clip indices (must match loaded mesh CAN file order) */
#define PLAYER_ANIM_IDLE 0
#define PLAYER_ANIM_WALK 1
#define PLAYER_ANIM_RUN 2
#define PLAYER_ANIM_TRANS_DUR FX_FROM_FLOAT(0.2f) /* idle<->walk blend duration */

typedef struct Player_s
{
    int ent_idx;          /* entity pool index (integer, -1 = not spawned) */
    i32 yaw;              /* world-space facing direction Q16.16 radians */
    i32 health;           /* 0..PLAYER_MAX_HEALTH Q16.16 */
    i32 stamina;          /* 0..PLAYER_MAX_STAMINA Q16.16 */
    int slot_q;           /* quickslot Q item id (integer, -1 = empty) */
    int slot_e;           /* quickslot E item id (integer, -1 = empty) */
    int anim_state;       /* PLAYER_ANIM_IDLE / PLAYER_ANIM_WALK (integer) */
    bool controls_active; /* whether can be controlled or not */
} Player;

extern Player g_player;

/**
 * @brief Initialise the player from an already-spawned entity index.
 *        Registers the think callback that drives input each frame.
 * @param ent_idx Active entity pool index with type ENT_PLAYER
 */
void player_init(int ent_idx);

#endif /* PLAYER_H */
