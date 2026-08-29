#include "flevel.h"
#include <stdio.h>
#include <string.h>
#include "CORE/CYBER.H"
#include "CORE/MMATERL.H"
#include "CORE/MTEXTUR.H"
#include "fentity.h"
#include "fplayr.h"

const char *MAP_BARRACKS =
    "########################"
    "#..E.m...######..E.m...#"
    "#........##==##........#"
    "#....B...##FF##...B....#"
    "#........##==##........#"
    "#......................#"
    "#.........!!!!.........#"
    "#......................#"
    "#####/############/#####"
    "#.........+............#"
    "#....T....+....G.......#"
    "#.........+............#"
    "#~~~~.....+......::::..#"
    "#~~~~............::::..#"
    "####/##############^####"
    "#E.......#.............#"
    "#...P..../.............#"
    "#........#....S........#"
    "####$#####.............#"
    "#........#.............#"
    "#........#.............#"
    "#........#.............#"
    "########################";

const char *MAP_RATIONBLOCK =
    "########################"
    "#NNN:#NNN:#NNN:#....L.*#"
    "#W...#W...#W...#.......#"
    "#....#....#....#.......#"
    "##/####/####/####*######"
    "#..m...................#"
    "#..~~~~....~~~~....Q...#"
    "#..~~~~....~~~~........#"
    "#..~~~~....~~~~........#"
    "####/################/##"
    "#::::#........#::::#W..#"
    "#::::#..B.....#::::#...#"
    "#N.W.#........#N...#...#"
    "##/####/########/####/##"
    "#......................#"
    "#.....E.......m........#"
    "#......................#"
    "########/#######+#######"
    "#P.....#.......#..W!!!!#"
    "#......#..M....#.......#"
    "#......#.......#.......#"
    "#......#.......#...$...#"
    "#......#.......#.......#"
    "########################";

const char *MAP_GENERATOR =
    "########################"
    "#t*....#~~~~~~#....E...#"
    "#......#~~~~~~#........#"
    "#......#~~~~~~#........#"
    "##/#####~~~~~~#####/####"
    "#......................#"
    "#..........LL..........#"
    "#..........LL..........#"
    "#..........ee..........#"
    "#......................#"
    "#....E............4....#"
    "#......................#"
    "####/##############/####"
    "#P.....#........#%%%%%%#"
    "#......#...B....#..O.O.#"
    "#......#........#%%%%%%#"
    "#......#....C...#!.....#"
    "########/########/######"
    "#~~~~~~#........#~~~~~~#"
    "#~~~~~~#...V....#~~~~~~#"
    "#~~~~~~#...e....#~~~~~~#"
    "#......#........#......#"
    "#..$...#........#......#"
    "########################";

void flevel_init(void)
{
    // Initialize level resources
}

void flevel_spawn_entities_from_grid(const char *grid, const char *mapName)
{
    // Iterate over 24x24 grid and spawn entities based on characters
    int row = 0;
    int col = 0;
    for (int i = 0; grid[i] != '\0'; i++)
    {
        char c = grid[i];

        // Skip newlines if any
        if (c == '\n' || c == '\r')
            continue;

        // Calculate world position based on grid (assuming each tile is 1 world unit, i.e., FX_ONE)
        // or whatever CyberVGA's scale is. Usually 1.0 or 2.0. Let's assume 1.0
        Vec4 pos;
        pos.x = FX_FROM_INT(col);
        pos.y = 0; // Ground level
        pos.z = FX_FROM_INT(row);
        pos.w = FX_ONE;

        switch (c)
        {
        case 'P':
            // Set player start
            fplayer_set_start_pos(pos);
            break;
        case 'E':
            // Spawn Enforcer
            fentity_spawn(ENT_TYPE_ENFORCER_F, pos);
            break;
        case 'm':
            // Spawn Male Enforcer
            fentity_spawn(ENT_TYPE_ENFORCER_M, pos);
            break;
        case '4':
            // Spawn Unit 4
            fentity_spawn(ENT_TYPE_UNIT4, pos);
            break;
        case 'B':
            // Spawn Broadcast (Radio)
            fentity_spawn(ENT_TYPE_RADIO, pos);
            break;
        case 'L':
            // L is Forced Labourer in Generator, but physical ledger in Ration Block
            if (mapName && strstr(mapName, "GENERATOR"))
            {
                // Spawn Boxer (Forced Labourer)
                fentity_spawn(ENT_TYPE_BOXER, pos);
            }
            break;
        case 'V':
            fentity_spawn(ENT_TYPE_VAN, pos);
            break;
        case 'M':
            fentity_spawn(ENT_TYPE_MEDKIT, pos);
            break;
        case 'F':
            fentity_spawn(ENT_TYPE_FACE, pos);
            break;
        case 'S':
        case 'G':
        case '!':
        case '*':
        case 'X':
        case 'I':
        case 'O':
        case 'C':
        case 'H':
        case 'N':
        case '$':
        case 'e':
        case 't':
        case 'r':
        case 'W':
        case 'Q':
        case '+':
            // Other entities/triggers
            break;
        default:
            break;
        }

        col++;
        if (col >= 24)
        {
            col = 0;
            row++;
        }
    }
}

void flevel_load(const char *mapName)
{
    console_log("Loading map: %s\n", mapName);

    // CyberVGA's engine_load_world handles both .CWR and .COC internally
    if (!engine_load_world(mapName))
    {
        console_log("Failed to load map: %s\n", mapName);
    }

    // Despawn old entities and spawn new ones from the map file
    engine_despawn_entities();
    engine_spawn_world_entities();

    // Now spawn entities from our ascii grid definitions
    if (strstr(mapName, "BARRACKS") || strstr(mapName, "HUB"))
    {
        flevel_spawn_entities_from_grid(MAP_BARRACKS, mapName);
    }
    else if (strstr(mapName, "RATION") || strstr(mapName, "MAP01") || strstr(mapName, "BLOCK"))
    {
        flevel_spawn_entities_from_grid(MAP_RATIONBLOCK, mapName);
    }
    else if (strstr(mapName, "GENERATOR") || strstr(mapName, "MAP02"))
    {
        flevel_spawn_entities_from_grid(MAP_GENERATOR, mapName);
    }
}

int flevel_is_in_warm_light(Vec4 position)
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

void flevel_update(void)
{
    // Level specific update logic (e.g., hot-swapping wall textures based on Loyalty/Doubt)
    // Managed externally via game state transitions, calling level_swap_wall_texture
    // TODO: Manage WARM vs COLD lighting logic based on spatial zones
}

void flevel_draw(Camera *cam, Surface8 *surf, const ClipRect *clip_rect)
{
    // Render the loaded world (brushes, point lights, entities) via CyberVGA
    engine_render(cam, surf, clip_rect);
}
