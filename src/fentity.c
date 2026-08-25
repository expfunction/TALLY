#include "fentity.h"
#include "GAME\GAME.H"

#define MAX_ENTITIES_T 128

static FEntity g_entities[MAX_ENTITIES_T];

void fentity_init(void)
{
    FEntity *first = &g_entities[0];
    first = (FEntity *)malloc(sizeof(FEntity) * MAX_ENTITIES_T);
}

void fentity_update(void)
{
    // Update AI state machines
}

void fentity_draw(void)
{
    // Render entities
}
