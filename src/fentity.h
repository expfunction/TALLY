#ifndef FENTITY_H
#define FENTITY_H

#include "CORE/CYBER.H"

typedef enum
{
    ENT_TYPE_ENFORCER,
    ENT_TYPE_UNIT4,
    ENT_TYPE_BOXER,
    ENT_TYPE_RADIO,
    ENT_TYPE_MEDKIT,
    ENT_TYPE_VAN,
    ENT_TYPE_FACE,
    ENT_TYPE_EXTRACTION,
    ENT_TYPE_PROJECTILE,
    ENT_TYPE_RATHOLE
} FEntityType;

typedef enum
{
    STATE_IDLE,
    STATE_PATROL,
    STATE_CHASE,
    STATE_ATTACK,
    STATE_BETRAYAL // Unit 4 specific
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

    // Sprite
    int sprite_id;
} FEntity;

void fentity_init(void);
void fentity_update(void);
void fentity_draw(Camera *cam, Surface8 *surf, const ClipRect *clip_rect);
void fentity_spawn(FEntityType type, Vec4 pos);
FEntity *fentity_get_all(void);

#endif
