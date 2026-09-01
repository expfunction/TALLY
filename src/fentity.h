/* Copyright (c) 2026 Burak Yazar */

#ifndef FENTITY_H
#define FENTITY_H

#include "CORE/CYBER.H"

#define MAX_CHAR_SPRITES 64

typedef enum
{
    ENT_TYPE_ENFORCER_F, /* Female enforcer */
    ENT_TYPE_ENFORCER_M, /* Male enforcer */
    ENT_TYPE_UNIT4,
    ENT_TYPE_BOXER,
    ENT_TYPE_RADIO,
    ENT_TYPE_MEDKIT,
    ENT_TYPE_VAN,
    ENT_TYPE_FACE,
    ENT_TYPE_EXTRACTION,
    ENT_TYPE_PROJECTILE,
    ENT_TYPE_RATHOLE,
    ENT_TYPE_DOOR,
    ENT_TYPE_DOOR_LOCKED,
    ENT_TYPE_LEDGER,     /* Physical ration ledger */
    ENT_TYPE_WORKER,     /* Civilian ration worker */
    ENT_TYPE_TERMINAL,   /* Directorate continuity terminal */
    ENT_TYPE_AMMO,       /* Surplus ammo pack / dropped rounds */
    ENT_TYPE_LIGHT       /* Ceiling light fixture prop */
} FEntityType;

typedef enum
{
    STATE_IDLE,
    STATE_PATROL,
    STATE_CHASE,
    STATE_ATTACK,
    STATE_HIT,
    STATE_BETRAYAL, // Unit 4 specific
    STATE_CLOSED,
    STATE_OPEN,
    STATE_OPENING,
    STATE_CLOSING
} FEntityState;

typedef struct
{
    int id;
    int active;
    FEntityType type;
    FEntityState state;

    // Spatial
    Vec4 pos;
    Vec3 rot; // Q16.16 Pitch, Yaw, Roll

// Stats
    i32 health;
    i32 speed;
    i32 timer; // Used for states/animations
} FEntity;

void fentity_init(void);
void fentity_clear(void);
void fentity_update(void);
void fentity_draw(Camera *cam, Surface8 *surf, const ClipRect *clip_rect);
void fentity_collide_world(FEntity *e);
FEntity *fentity_spawn(FEntityType type, Vec4 pos);
FEntity *fentity_get_all(void);
Sprite *fentity_get_sprite(FEntityType type);

#endif
