#ifndef ENTITY_H
#define ENTITY_H

typedef enum {
    ENT_TYPE_ENFORCER,
    ENT_TYPE_UNIT4,
    ENT_TYPE_BOXER,
    ENT_TYPE_RADIO
} EntityType;

typedef struct {
    int id;
    int active;
    EntityType type;
    // Spatial data, state data
} Entity;

void entity_init(void);
void entity_update(void);
void entity_draw(void);

#endif
