/* Copyright (c) 2026 Burak Yazar */

#ifndef SAVE_H
#define SAVE_H

#include "CORE/CENG.H"

typedef enum
{
    PMODE_REPLY,
    PMODE_IMITATE_AUDIO,
    PMODE_UNSEEN_RELOCATE,
    PMODE_FOLLOW_GATES,
    PMODE_BOSS,
    PMODE_WITHDRAWN
} PresenceMode;

typedef struct
{
    int scene_id;             /* 0-9 */
    int save_charges;         /* 3 -> 0 */
    int dusk_lock;            /* false/true */
    PresenceMode p_mode;      /* Current Presence behavior */
    int presence_anchor;      /* Current anchor ID */
    int gate_progress;        /* 0-3 */
    int boss_phase;           /* 0-3 */
    int imitation_level;      /* 0:Intact, 1:Step, 2:Breath, 3:Form */
    i32 pulse_cooldown;       /* Cooldown timer for Breath Pulse */
    int fourth_rule_revealed; /* false/true */
    int failure_seen;         /* false/true */
} DemoState;

extern DemoState g_demo;

/**
 * @brief Saves the given DemoState to a file (SAVE01.DAT).
 * @return 1 on success, 0 on failure.
 */
int save_game_state(const DemoState *state);

/**
 * @brief Loads the DemoState from a file (SAVE01.DAT).
 * @return 1 on success, 0 on failure.
 */
int load_game_state(DemoState *state);

#endif /* SAVE_H */
