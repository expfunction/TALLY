/* Copyright (c) 2026 Burak Yazar */

#ifndef PRESENCE_H
#define PRESENCE_H

#include "CORE/CENG.H"
#include "GAME/STMACH.H"

/* Presence Events */
#define PEV_INTERACT 1
#define PEV_SCENE_ADVANCE 2
#define PEV_TIMER_DONE 3
#define PEV_RUSH_DONE 4
#define PEV_PULSE 5

/* Presence States mapping directly to PresenceMode */
#define PST_REPLY 0
#define PST_IMITATE_AUDIO 1
#define PST_UNSEEN_RELOCATE 2
#define PST_FOLLOW_GATES 3
#define PST_BOUNDARY_WAIT 4
#define PST_BOSS_MARK 5
#define PST_BOSS_RUSH 6
#define PST_BOSS_RECOVER 7
#define PST_BOSS_RELOCATE 8
#define PST_WITHDRAWN 9
#define PST_FAIL_FORM 10

/**
 * @brief Initialize the Presence entity and state machine.
 */
void presence_init(void);

/**
 * @brief Update the Presence entity.
 * @param dt Time delta in Q16.16
 */
void presence_update(i32 dt);

/**
 * @brief Fire an event to the Presence state machine.
 * @param ev The event ID
 */
void presence_fire_event(SmEv ev);

#endif /* PRESENCE_H */
