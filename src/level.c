#include "level.h"
#include <stdio.h>
#include "CORE/CYBER.H"
#include "CORE/MMATERL.H"
#include "CORE/MTEXTUR.H"

void level_init(void)
{
    // TODO: Add ascii level string here.
}

void level_load(const char *mapName)
{
    printf("Loading map: %s\n", mapName);

    // CyberVGA's engine_load_world handles both .CWR and .COC internally
    if (!engine_load_world(mapName))
    {
        printf("Failed to load map: %s\n", mapName);
    }

    // Despawn old entities and spawn new ones from the map file
    engine_despawn_entities();
    engine_spawn_world_entities();
}

void level_swap_wall_texture(const char *matName, const char *texName)
{
    int mat_idx = materials_find_by_name(matName);
    if (mat_idx >= 0)
    {
        int tex_idx = textures_find_by_name(m_textures, MAX_TEXTURES, texName);
        if (tex_idx >= 0)
        {
            m_mat_texture_id[mat_idx] = tex_idx;
        }
    }
}

int level_is_in_warm_light(Vec4 position)
{
    // Find closest light in g_world
    PointLight *closest = NULL;
    i32 min_dist = 0x7FFFFFFF;

    for (int i = 0; i < g_world.num_pointLights; i++)
    {
        PointLight *pl = &g_world.pointLights[i];
        i32 dist = vec4_dist(&position, &pl->position);

        if (dist < min_dist)
        {
            min_dist = dist;
            closest = pl;
        }
    }

    if (closest)
    {
        // Assume YELLOW/RED are WARM lights, others (WHITE/BLUE) are COLD
        if (closest->color == LCOLOR_YELLOW || closest->color == LCOLOR_RED)
        {
            return 1;
        }
    }

    return 0;
}

void level_update(void)
{
    // Level specific update logic (e.g., hot-swapping wall textures based on Loyalty/Doubt)
    // Managed externally via game state transitions, calling level_swap_wall_texture
    // TODO: Manage WARM vs COLD lighting logic based on spatial zones
}

void level_draw(Camera *cam, Surface8 *surf)
{
    // Render the loaded world (brushes, point lights, entities) via CyberVGA
    engine_render(cam, surf, NULL);
}
