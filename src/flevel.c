/* Copyright (c) 2026 Burak Yazar */

#include "flevel.h"
#include <stdio.h>
#include <string.h>
#include "CORE/CYBER.H"
#include "CORE/MMATERL.H"
#include "CORE/MTEXTUR.H"
#include "fentity.h"
#include "fplayr.h"
#include "fai.h"
#include "fgame.h"
#include "faudio.h"

const char *MAP_BARRACKS =
    "########################"
    "#........######........#"
    "#........##..##........#"
    "#....B...##FF##...B....#"
    "#........##..##........#"
    "#......................#"
    "#.........!!!!.........#"
    "#......................#"
    "#####/############/#####"
    "#......................#"
    "#....E.........m.......#"
    "#......................#"
    "#~~~~............::::..#"
    "#~~~~............::::..#"
    "####/##############^####"
    "#........#.............#"
    "#...P.4../.............#"
    "#........#....S........#"
    "####/#####.............#"
    "#...A....#.............#"
    "#........#.............#"
    "#........#.............#"
    "#........#.............#"
    "########################";

const char *MAP_RATIONBLOCK =
    "########################"
    "#NNN:#NNN:#NNN:#....L.*#"
    "#W...#W...#W...#.......#"
    "#A...#....#....#.......#"
    "##/####/####/####*######"
    "#..E...................#"
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
    "####/######/####$#######"
    "#...P..#.......#..W!!!!#"
    "#......#..M....#.......#"
    "#......#.......#.......#"
    "#......#.......#.......#"
    "#......#.......#.......#"
    "########################";

const char *MAP_GENERATOR =
    "########################"
    "##r....##~~~~~##...E...#"
    "##.....##~~~~~##.......#"
    "##.....##~~~~~##.......#"
    "###/#####~~~~~#####/####"
    "###.###############.####"
    "##.....................#"
    "##.........L...........#"
    "##.....................#"
    "##.........ee..........#"
    "##...E............E....#"
    "##.....................#"
    "####.###################"
    "####/###################"
    "##.....##.......########"
    "##..P..##..B..A.########"
    "##.....##.......########"
    "##....../.......##.....#"
    "##.....##..V....##.....#"
    "##.....##..e....##.....#"
    "##.....##...C...##.....#"
    "##.....##.......##.....#"
    "#######.##/#####.#######"
    "########################";

void flevel_init(void)
{
    // Initialize level resources
}

void flevel_spawn_entities_from_grid(const char *grid, const char *mapName)
{
    // Keep AI map up to date with the newly loaded grid
    fai_update_map(grid);

    // Iterate over 24x24 grid and spawn entities based on characters
    int row = 0;
    int col = 0;
    for (int i = 0; grid[i] != '\0'; i++)
    {
        char c = grid[i];

        // Skip newlines if any
        if (c == '\n' || c == '\r')
            continue;

        // Calculate world position based on grid. Tile size is 2.0 world units.
        // Map should be generated at +Z (Blender -Y).
        Vec4 pos;
        pos.x = FX_FROM_INT(col * 2);
        pos.y = (FX_HALF * 3 / 2); // Ground level for sprites
        pos.z = FX_FROM_INT(row * 2);
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
        case 'e':
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
                fentity_spawn(ENT_TYPE_BOXER, pos);
            }
            else
            {
                fentity_spawn(ENT_TYPE_LEDGER, pos);
            }
            break;
        case 'W':
            fentity_spawn(ENT_TYPE_WORKER, pos);
            break;
        case 'C':
            fentity_spawn(ENT_TYPE_TERMINAL, pos);
            break;
        case 'V':
        {
            pos.y = 0;
            fentity_spawn(ENT_TYPE_VAN, pos);
            break;
        }
        case '^':
        case '*':
        case 't':
            fentity_spawn(ENT_TYPE_EXTRACTION, pos);
            break;
        case 'r':
            fentity_spawn(ENT_TYPE_RATHOLE, pos);
            fentity_spawn(ENT_TYPE_EXTRACTION, pos);
            break;
        case '/':
        case '$':
        {
            FEntity *door = fentity_spawn((c == '$') ? ENT_TYPE_DOOR_LOCKED : ENT_TYPE_DOOR, pos);
            if (door)
            {
                // Door mesh is centered at its origin (Z=0 in Blender, Y=0 in CVGA).
                door->pos.y = 0;

                // Check if passage is vertical or horizontal
                // grid is 24x24 (1d array)
                int left_idx = (row * 24) + (col - 1);
                int right_idx = (row * 24) + (col + 1);
                int up_idx = ((row - 1) * 24) + col;
                int down_idx = ((row + 1) * 24) + col;

                int has_h_wall = 0;
                int has_v_wall = 0;

                if (col > 0 && (grid[left_idx] == '#' || grid[left_idx] == '~' || grid[left_idx] == '%' || grid[left_idx] == ':'))
                    has_h_wall = 1;
                if (col < 23 && (grid[right_idx] == '#' || grid[right_idx] == '~' || grid[right_idx] == '%' || grid[right_idx] == ':'))
                    has_h_wall = 1;

                if (row > 0 && (grid[up_idx] == '#' || grid[up_idx] == '~' || grid[up_idx] == '%' || grid[up_idx] == ':'))
                    has_v_wall = 1;
                if (row < 23 && (grid[down_idx] == '#' || grid[down_idx] == '~' || grid[down_idx] == '%' || grid[down_idx] == ':'))
                    has_v_wall = 1;

                // If wall is vertical (North-South, wall above/below and not left/right),
                // passage goes East-West, so door must span along Z (rot.y = pi/2).
                // Otherwise wall is horizontal (East-West), passage goes North-South, door spans along X (rot.y = 0).
                if (has_v_wall && !has_h_wall)
                {
                    door->rot.y = FX_FROM_FLOAT(1.570796f); // pi/2
                }
                else
                {
                    door->rot.y = 0;
                }
            }
            break;
        }
        case 'M':
            fentity_spawn(ENT_TYPE_MEDKIT, pos);
            break;
        case 'A':
            pos.y = FX_FROM_FLOAT(0.25f);
            fentity_spawn(ENT_TYPE_AMMO, pos);
            break;
        case 'F':
            fentity_spawn(ENT_TYPE_FACE, pos);
            break;
        case 'S':
        case 'G':
        case '!':
        case 'X':
        case 'I':
        case 'O':
        case 'H':
        case 'N':
        case 'Q':
        case '+':
            // Other static scenery markers/triggers
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
    fentity_clear();

    // Now spawn entities from our ascii grid definitions
    if (strstr(mapName, "BARRACKS") || strstr(mapName, "HUB"))
    {
        if (!fai_init_map(MAP_BARRACKS))
        {
            console_log("Error initializing Astar grid for BARRACKS");
            return;
        }
        flevel_spawn_entities_from_grid(MAP_BARRACKS, mapName);
        if (g_mission_progress >= 2)
        {
            level_swap_wall_texture("ASSTS\\MESH\\BARRACKS\\TEXTURES\\Mat_Wall_Commandment_B.RAW");
        }
    }
    else if (strstr(mapName, "RATION") || strstr(mapName, "MAP01") || strstr(mapName, "BLOCK"))
    {
        if (!fai_init_map(MAP_RATIONBLOCK))
        {
            console_log("Error initializing Astar grid for RATION");
            return;
        }
        flevel_spawn_entities_from_grid(MAP_RATIONBLOCK, mapName);
    }
    else if (strstr(mapName, "GENERATOR") || strstr(mapName, "MAP02"))
    {
        if (!fai_init_map(MAP_GENERATOR))
        {
            console_log("Error initializing Astar grid for GENERATOR");
            return;
        }
        flevel_spawn_entities_from_grid(MAP_GENERATOR, mapName);
    }

    // Spawn ceiling light props at all point light positions
    for (int i = 0; i < g_world.num_pointLights; i++)
    {
        Vec4 light_pos = g_world.pointLights[i].position;
        light_pos.y = FX_FROM_FLOAT(2.0f) - FX_FROM_FLOAT(0.375f);
        light_pos.w = FX_ONE;
        fentity_spawn(ENT_TYPE_LIGHT, light_pos);
    }
}

void level_swap_wall_texture(const char *tex_path)
{
    if (g_world.num_meshes > 0)
    {
        MeshCMS *mesh = &g_world.meshes[0].mesh;
        // Specifically swap the Civic Record / Commandment wall slots (indices 9 and 10)
        int target_slots[] = {9, 10};
        for (int s = 0; s < 2; s++)
        {
            int i = target_slots[s];
            if (i < mesh->texture_count && mesh->textures[i])
            {
                u32 tw = mesh->texture_w[i];
                u32 th = (tw == 128) ? 64 : ((tw == 64) ? 64 : 128);
                load_raw8(tex_path, mesh->textures[i], tw, th);
            }
        }
        audio_play_sfx(SFX_WALL_SWAP);
    }
}

void level_swap_wall_ending(int ending_id)
{
    if (ending_id == 1)
    {
        level_swap_wall_texture("ASSTS\\MESH\\BARRACKS\\TEXTURES\\Mat_Wall_Standard.RAW");
    }
    else if (ending_id == 2)
    {
        level_swap_wall_texture("ASSTS\\MESH\\BARRACKS\\TEXTURES\\Mat_Wall_Civic_Record.RAW");
    }
    else if (ending_id == 3)
    {
        level_swap_wall_texture("ASSTS\\MESH\\BARRACKS\\TEXTURES\\Mat_Wall_Face.RAW");
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
}

void flevel_draw(Camera *cam, Surface8 *surf, const ClipRect *clip_rect)
{
    // Render the loaded world (brushes, point lights, entities) via CyberVGA
    engine_render(cam, surf, clip_rect);
}
