#ifndef FENTITY_H
#define FENTITY_H

typedef enum
{
    ENT_TYPE_ENFORCER,
    ENT_TYPE_UNIT4,
    ENT_TYPE_BOXER,
    ENT_TYPE_RADIO
} FEntityType;

typedef struct
{
    int id;
    int active;
    FEntityType type;
    // Spatial data, state data
} FEntity;

void fentity_init(void);
void fentity_update(void);
void fentity_draw(void);

#endif
