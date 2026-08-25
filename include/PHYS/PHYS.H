/* Copyright (c) 2026 Burak Yazar */

#ifndef PHYS_H
#define PHYS_H

#include "../CORE/TYPES/WORLD.H"
#include "../RNDR/CAMER.H"

/* Forward-declare Entity (defined in GAME/ENTITY.H) */
typedef struct Entity_s Entity;

/* Physics component (8 bytes, gated by ENT_F_PHYS) */
typedef struct EntPhys_s
{
    i32 radius;   /* sphere radius Q16.16 */
    i32 height;   /* capsule height Q16.16 (0 = sphere) */
    u8 col_mask;  /* collision layer mask */
    u8 on_ground; /* true if resting on a floor surface */
} EntPhys;

/*
 * Collision hit info passed to on_hit callbacks.
 * normal points away from the obstacle (push-out direction for self).
 * other_idx is -1 for wall hits, otherwise the entity pool index.
 */
typedef struct EntHitInfo_s
{
    int self_idx;  /* pool index of the entity receiving this callback */
    int other_idx; /* pool index of the other entity, or -1 for a wall */
    Vec3 normal;   /* push-out normal (unit, Q16.16) */
    i32 penetr;    /* penetration depth at moment of hit (Q16.16) */
} EntHitInfo;

typedef void (*EntHitFn)(Entity *self, const EntHitInfo *hit);

/* Collision layer bits */
#define COL_WORLD 0x01
#define COL_ENTITIES 0x02

/* Physics constants */
#define PHYS_GRAVITY FX_FROM_FLOAT(-9.8f)
#define PHYS_CAM_RADIUS FX_FROM_FLOAT(0.8f)
#define PHYS_DEFAULT_RADIUS FX_FROM_FLOAT(0.3f)
#define PHYS_EPSILON 64 /* ~0.001 in Q16.16 */

/* Entity flags for physics (fit in existing u16 flags bitfield) */
#define ENT_F_PHYS 0x0080
#define ENT_F_GRAVITY 0x0100

/**
 * @brief Initialize the physics system. Call once at startup.
 */
void phys_init(void);

/**
 * @brief Per-frame physics update for all entities with ENT_F_PHYS.
 *        Applies gravity, resolves entity-world and entity-entity collisions.
 * @param dt delta time Q16.16
 * @param world pointer to the world structure
 */
void phys_update(i32 dt, const World *world);

/**
 * @brief Set physics component on an entity
 * @param idx entity pool index
 * @param radius sphere collider radius Q16.16
 * @param col_mask collision layer bitmask (COL_WORLD, COL_ENTITIES)
 */
void phys_set(int idx, i32 radius, i32 height, u8 col_mask);

/**
 * @brief Push camera out of solid walls of brushes
 * @param cam pointer to the camera
 * @param world pointer to the world structure
 */
void phys_collide_camera(Camera *cam, const World *world);

#endif /* PHYS_H */
