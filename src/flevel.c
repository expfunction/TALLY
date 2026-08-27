#include "flevel.h"
#include <stdio.h>
#include <string.h>
#include "CORE/CYBER.H"
#include "CORE/MMATERL.H"
#include "CORE/MTEXTUR.H"
#include "fentity.h"
#include "fplayr.h"

/**
 * @brief Legend:
 * F: The Face mural (never dies)
 * !: WARM aisle (range 5)
 * G: Pistol (WARM, range 4)
 * P: Start (COLD yard leak, range 3)
 * S: Scrap in shadow
 */
const char *MAP_BARN =
    "########################"
    "#........######........#"
    "#........##==##........#"
    "#....R...##FF##...R....#"
    "#........##==##........#"
    "#.......................#"
    "#.........!!!!..........#"
    "#.......................#"
    "#####/############/#####"
    "#.........+.............#"
    "#....T....+....G........#"
    "#.........+.............#"
    "#~~~~.....+......::::...#"
    "#~~~~............::::...#"
    "####/##############^####"
    "#........#..............#"
    "#...P..../..............#"
    "#........#..............#"
    "#........#....S.........#"
    "####$#####..............#"
    "#........#..............#"
    "#........#..............#"
    "#........#..............#"
    "########################";

/**
 * @brief Legend:
 * S: Ledger (COLD, range 2)
 * D: Grain hoard (WARM, range 6)
 * R: Radio
 * D: Enforcer (WARM collar, range 1)
 * !: Last worker pen (COLD, range 3)
 * M: Medkit Trap (Bright)
 */
const char *MAP_HENHOUSE =
    "########################"
    "#NNN:#NNN:#NNN:#....S.*#"
    "#H...#H...#H...#.......#"
    "#....#....#....#.......#"
    "##/####/####/####*######"
    "#......................#"
    "#..~~~~....~~~~....D...#"
    "#..~~~~....~~~~........#"
    "#..~~~~....~~~~........#"
    "####/################/##"
    "#::::#........#::::#H..#"
    "#::::#..R.....#::::#...#"
    "#N.H.#........#N...#...#"
    "##/####/########/####/##"
    "#......................#"
    "#.....D................#"
    "#......................#"
    "########/#######+#######"
    "#P.....#.......#..H!!!!#"
    "#......#..M....#.......#"
    "#......#.......#.......#"
    "#......#.......#...$...#"
    "#......#.......#.......#"
    "########################";

/**
 * @brief Legend:
 * S: Cold archives
 * X: Stove (Burning truth makes gold)
 * P: Start
 * R: Radio
 * M: Trap; K: Pass bait (WARM)
 * !: Trigger (Renderer hitches 1 frame)
 * C: Comrades door (Brightest light)
 * O: Officers behind glass (Unlit)
 */
const char *MAP_MINISTRY =
    "########################"
    "#S....#::::#....S#^^^^# "
    "#.....#:X::#.....#^^^^# "
    "#..I..#::::#..I..#^^^^# "
    "##/#####/#####/###^#### "
    "#....................4.#"
    "#....................P.#"
    "#.........R............#"
    "####/##############/####"
    "#......#........#......#"
    "#..S...#...M....#..K...#"
    "#......#........#......#"
    "##/#####!!!!!!!!#/######"
    "#......#........#......#"
    "#..D...#........#..D...#"
    "#......#........#......#"
    "########/######C########"
    "#~~~~~~#........#%%%%%%#"
    "#~~~~~~#...$....#..O.O.#"
    "#~~~~~~#........#%%%%%%#"
    "#..I...#........#!.....#"
    "#......#........#......#"
    "#......#........#......#"
    "########################";

/**
 * @brief Legend:
 * r: Rat hole (B ending)
 * B: Boxer (COLD work-lamp, range 4)
 * V: Recycling Van (WARM, range 6)
 */
const char *MAP_MILL =
    "########################"
    "#r*....#~~~~~~#....D...#"
    "#......#~~~~~~#........#"
    "#......#~~~~~~#........#"
    "##/#####~~~~~~#####/####"
    "#......................#"
    "#..........BB..........#"
    "#..........BB..........#"
    "#..........ee..........#"
    "#......................#"
    "#....D............4....#"
    "#......................#"
    "####/##############/####"
    "#P.....#........#%%%%%%#"
    "#......#...R....#..O.O.#"
    "#......#........#%%%%%%#"
    "#......#........#!.....#"
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

void flevel_spawn_entities_from_grid(const char *grid)
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
            // Set player start and spawn extraction zone
            fplayer_set_start_pos(pos);
            fentity_spawn(ENT_TYPE_EXTRACTION, pos);
            break;
        case 'D':
            // Spawn Enforcer
            fentity_spawn(ENT_TYPE_ENFORCER, pos);
            break;
        case '4':
            // Spawn Unit 4
            fentity_spawn(ENT_TYPE_UNIT4, pos);
            break;
        case 'R':
            // Spawn Radio
            fentity_spawn(ENT_TYPE_RADIO, pos);
            break;
        case 'B':
            // Spawn Boxer
            fentity_spawn(ENT_TYPE_BOXER, pos);
            break;
        case 'V':
            fentity_spawn(ENT_TYPE_VAN, pos);
            break;
        case 'r':
            fentity_spawn(ENT_TYPE_RATHOLE, pos);
            break;
        case '+':
            fentity_spawn(ENT_TYPE_MEDKIT, pos);
            break;
        case 'F':
            fentity_spawn(ENT_TYPE_FACE, pos);
            break;
        case 'S':
        case 'G':
        case 'M':
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
    if (strstr(mapName, "BARN") || strstr(mapName, "HUB"))
    {
        flevel_spawn_entities_from_grid(MAP_BARN);
    }
    else if (strstr(mapName, "HENHOUSE") || strstr(mapName, "MAP01") || strstr(mapName, "TENEMENTS"))
    {
        flevel_spawn_entities_from_grid(MAP_HENHOUSE);
    }
    else if (strstr(mapName, "MINISTRY") || strstr(mapName, "MAP02") || strstr(mapName, "ARCHIVES"))
    {
        flevel_spawn_entities_from_grid(MAP_MINISTRY);
    }
    else if (strstr(mapName, "MILL") || strstr(mapName, "MAP03") || strstr(mapName, "GENERATOR"))
    {
        flevel_spawn_entities_from_grid(MAP_MILL);
    }

    // Hot-swap Commandment Wall texture if in Hub based on endings / doubt
    if (strstr(mapName, "BARN") || strstr(mapName, "HUB"))
    {
        int face_tex_id = textures_find_by_name(m_textures, MAX_TEXTURES, "Mat_Wall_Face_Mural");
        if (face_tex_id >= 0) {
            if (g_doubt >= 3) {
                // Change to TRUST NO ONE or equivalent
                int trust_id = textures_find_by_name(m_textures, MAX_TEXTURES, "WALLRED");
                if (trust_id >= 0) m_textures[face_tex_id] = m_textures[trust_id];
            } else if (g_loyalty >= 3) {
                // You become the new face (just keep it or change it)
                int new_face = textures_find_by_name(m_textures, MAX_TEXTURES, "FACE");
                if (new_face >= 0) m_textures[face_tex_id] = m_textures[new_face];
            }
        }
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
    // Track WARM vs COLD lighting logic based on spatial zones
    static int light_timer = 0;
    light_timer++;
    
    if (light_timer >= 150) // Every 5 seconds (assuming 30fps)
    {
        light_timer = 0;
        if (flevel_is_in_warm_light(g_player.w_pos))
        {
            g_loyalty++;
        }
        else
        {
            g_doubt++;
        }
    }
}

void flevel_draw(Camera *cam, Surface8 *surf, const ClipRect *clip_rect)
{
    // Render the loaded world (brushes, point lights, entities) via CyberVGA
    engine_render(cam, surf, clip_rect);
}
