#include "entity.h"

#define MAX_ENTITIES 128

static Entity g_entities[MAX_ENTITIES];

void entity_init(void)
{
    for (int i = 0; i < MAX_ENTITIES; i++) {
        g_entities[i].active = 0;
    }
}

void entity_update(void)
{
    // Update AI state machines
}

void entity_draw(void)
{
    // Render entities
}
