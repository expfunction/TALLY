#include "entity.h"

#define MAX_ENTITIES 128

static Entity g_entities[MAX_ENTITIES];

void entity_init(void)
{
    Entity *first = &g_entities[0];
    first = (Entity *)malloc(sizeof(Entity) * MAX_ENTITIES);
}

void entity_update(void)
{
    // Update AI state machines
}

void entity_draw(void)
{
    // Render entities
}
