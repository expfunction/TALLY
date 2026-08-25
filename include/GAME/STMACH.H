/* Copyright (c) 2026 Burak Yazar */

/* GAME/STMACH.H — Generic finite state machine
 *
 * Event-driven + condition-based FSM with reentrancy protection.
 * Used for quests, dialogue flow, and animation state management.
 *
 * ---- Quick-start -------------------------------------------------------
 *  1. Define state ids as enum or #define (u8, 0-253).
 *     SM_NONE (0xFF) is the null sentinel; SM_ANY (0xFE) is wildcard source.
 *  2. Define event ids the same way.
 *     SM_EV_AUTO (0xFE) marks auto-transitions (no event required).
 *  3. Fill a static SmStateDef states[] array indexed by state id.
 *  4. Fill a static SmTrans    trans[]  transition table.
 *  5. Point a static SmDef at both arrays.
 *  6. sm_alloc(&def, INITIAL_STATE, ctx_ptr)  → id
 *  7. sm_update(id, dt) each frame, sm_fire(id, EV_X) for events.
 *
 * ---- Transition kinds --------------------------------------------------
 *  Event-driven:       { FROM, EV_X,        TO, NULL,   action }
 *  Guarded event:      { FROM, EV_X,        TO, guard,  action }
 *  Auto (per-frame):   { FROM, SM_EV_AUTO,  TO, cond,   action }
 *  Wildcard source:    { SM_ANY, EV_ABORT,  TO, NULL,   action }
 *
 * ---- Reentrancy --------------------------------------------------------
 *  Transitions fired inside callbacks (on_enter/on_exit/action) are
 *  deferred into a per-instance pending queue and drained after the
 *  current do_transition() returns.  The queue holds SM_PENDING_MAX
 *  events; excess events are silently dropped.
 *  sm_update() also enforces a hard cap of SM_MAX_TRANS_PER_FRAME total
 *  transitions per call to prevent infinite loops in condition chains.
 */
#ifndef STMACH_H
#define STMACH_H

#include "../CORE/TYPES/MATH.H"

/* ---- Limits ------------------------------------------------------------- */

#define SM_NONE ((u8)0xFF)       /* null state / no-event sentinel     */
#define SM_ANY ((u8)0xFE)        /* wildcard source state in trans[]   */
#define SM_EV_AUTO ((u8)0xFE)    /* marks auto (condition) transition  */
#define SM_MAX_POOL 16           /* max simultaneous instances          */
#define SM_PENDING_MAX 4         /* deferred-event queue per instance   */
#define SM_MAX_TRANS_PER_FRAME 8 /* hard cap on transitions per update  */

/* ---- Types -------------------------------------------------------------- */

typedef u8 SmSt; /* state identifier  (0-253; 0xFE = SM_ANY; 0xFF = SM_NONE) */
typedef u8 SmEv; /* event identifier  (0-253; 0xFE = SM_EV_AUTO)             */

/* Per-state callbacks — all optional (NULL = skip).
 *   on_enter  : fired on entry;  from == SM_NONE means initial entry via sm_alloc
 *   on_exit   : fired on exit, before on_enter of the next state
 *   on_update : called every frame; return event to fire, SM_NONE to stay put
 */
typedef void (*SmEnterFn)(int id, SmSt from, SmSt to, void *ctx);
typedef void (*SmExitFn)(int id, SmSt state, void *ctx);
typedef SmEv (*SmUpdateFn)(int id, SmSt state, i32 dt, void *ctx);

/* Guard / condition function.
 *   For event transitions (ev != SM_EV_AUTO): acts as a guard; transition
 *     fires only when cond() returns 1.  cond == NULL means always pass.
 *   For auto transitions (ev == SM_EV_AUTO): acts as trigger condition;
 *     checked every frame; transition fires when cond() returns 1.
 *     cond must be non-NULL for auto-transitions.
 */
typedef int (*SmCondFn)(int id, SmSt state, void *ctx);

/* Transition action — optional callback fired after state change but before
 * on_enter of the new state.  Receives the triggering event for context.
 */
typedef void (*SmTransFn)(int id, SmSt from, SmSt to, SmEv ev, void *ctx);

/* One row in the transition table */
typedef struct SmTrans_s
{
    SmSt from;        /* source state (SM_ANY = match any current state)   */
    SmEv ev;          /* event id, or SM_EV_AUTO for condition transitions  */
    SmSt to;          /* destination state                                  */
    SmCondFn cond;    /* guard (event) or trigger condition (auto)          */
    SmTransFn action; /* optional; called during transition (may be NULL)   */
} SmTrans;

/* Per-state definition entry */
typedef struct SmStateDef_s
{
    SmEnterFn on_enter;
    SmExitFn on_exit;
    SmUpdateFn on_update;
} SmStateDef;

/* State machine definition — static const data shared across instances */
typedef struct SmDef_s
{
    const SmStateDef *states; /* [num_states] indexed by state id       */
    u8 num_states;
    const SmTrans *trans; /* [num_trans] transition table           */
    u8 num_trans;

    /* Debug names — compiled out in release builds */
#ifndef RELEASE
    const char **state_names; /* [num_states], may be NULL                */
    const char **event_names; /* optional parallel event name table       */
#endif
} SmDef;

/* Runtime instance — allocated from the pool by sm_alloc */
typedef struct SmInst_s
{
    u8 active;  /* 1 = in use                  */
    SmSt state; /* current state id            */
    SmSt prev;  /* state before last transition*/
    i32 timer;  /* time in state (Q16.16 sec)  */
    const SmDef *def;
    void *ctx; /* caller context pointer      */

    /* Reentrancy */
    u8 in_trans;                  /* 1 while do_transition runs  */
    SmEv pending[SM_PENDING_MAX]; /* deferred events             */
    u8 pending_count;
} SmInst;

/* ---- Pool API ------------------------------------------------------------ */

/**
 * Allocate a new instance; fires on_enter for initial_state immediately.
 * Returns instance id (0..SM_MAX_POOL-1) or -1 if pool is full.
 */
int sm_alloc(const SmDef *def, SmSt initial_state, void *ctx);

/** Release instance; safe to call with id == -1 or already-free id. */
void sm_free(int id);

/**
 * Fire an event.  Walks the transition table for the first row where:
 *   - from matches current state (or SM_ANY)
 *   - ev matches the fired event (not SM_EV_AUTO)
 *   - cond is NULL or cond() returns 1
 * Performs the transition if found; otherwise the event is silently ignored.
 * If called inside a callback, the event is deferred to the pending queue.
 */
void sm_fire(int id, SmEv ev);

/**
 * Advance one frame.
 *   1. Increment timer by dt.
 *   2. Call on_update; fire the returned event (if != SM_NONE).
 *   3. Check auto-transition (SM_EV_AUTO) rows; fire the first that passes.
 * At most SM_MAX_TRANS_PER_FRAME total transitions are allowed per call.
 */
void sm_update(int id, i32 dt);

/** Convenience: calls sm_update(id, dt) for every active instance. */
void sm_update_all(i32 dt);

/** Current state, or SM_NONE if id is invalid. */
SmSt sm_state(int id);

/** Time in current state (Q16.16 seconds), or 0 if id is invalid. */
i32 sm_timer(int id);

/**
 * Direct instance pointer for inspection.  Returns NULL if id is invalid.
 * Do not cache across sm_alloc/sm_free calls.
 */
SmInst *sm_get(int id);

#endif /* STMACH_H */
