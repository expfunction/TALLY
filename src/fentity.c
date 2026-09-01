/* Copyright (c) 2026 Burak Yazar */

#include "fentity.h"
#include "fgame.h"
#include "fplayr.h"
#include "CORE/MTEXTUR.H"
#include "RNDR/SPRIT.H"
#include "MESH/CMS.H"
#include "RNDR/TRIMES.H"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "fai.h"
#include "faudio.h"
#include "fui.h"

#define MAX_ENTITIES_T 128

typedef struct
{
    FEntity *e;
    i32 dist;
} FEntitySortNode;

static int compare_entity_dist(const void *a, const void *b)
{
    FEntitySortNode *na = (FEntitySortNode *)a;
    FEntitySortNode *nb = (FEntitySortNode *)b;
    if (na->dist < nb->dist)
        return 1;
    if (na->dist > nb->dist)
        return -1;
    return 0;
}

static FEntity g_entities[MAX_ENTITIES_T];
static MeshCMS g_door_mesh_x;
static MeshCMS g_door_mesh_z;
static MeshCMS g_doorl_mesh_x;
static MeshCMS g_doorl_mesh_z;
static MeshCMS g_van_mesh;

static Vec4 s_chase_target[MAX_ENTITIES_T];
static i32 s_path_cooldown[MAX_ENTITIES_T];
static int s_last_px[MAX_ENTITIES_T];
static int s_last_py[MAX_ENTITIES_T];

void fentity_collide_world(FEntity *e)
{
    if (!e || !e->active)
        return;

    const i32 radius = FX_FROM_FLOAT(0.50f);
    int center_gx = (int)floorf(FX_TO_FLOAT(e->pos.x) / 2.0f + 0.5f);
    int center_gz = (int)floorf(FX_TO_FLOAT(e->pos.z) / 2.0f + 0.5f);

    // Multiple collision passes for corners
    for (int pass = 0; pass < 2; pass++)
    {
        for (int gz = center_gz - 1; gz <= center_gz + 1; gz++)
        {
            for (int gx = center_gx - 1; gx <= center_gx + 1; gx++)
            {
                int is_solid = 0;
                if (gx < 0 || gx >= MAP_WIDTH || gz < 0 || gz >= MAP_HEIGHT)
                {
                    is_solid = 1;
                }
                else if (g_map_grid[gz * MAP_WIDTH + gx])
                {
                    is_solid = 1;
                }

                if (is_solid)
                {
                    i32 tile_min_x = FX_FROM_INT(gx * 2 - 1);
                    i32 tile_max_x = FX_FROM_INT(gx * 2 + 1);
                    i32 tile_min_z = FX_FROM_INT(gz * 2 - 1);
                    i32 tile_max_z = FX_FROM_INT(gz * 2 + 1);

                    i32 cx = e->pos.x;
                    if (cx < tile_min_x)
                        cx = tile_min_x;
                    else if (cx > tile_max_x)
                        cx = tile_max_x;

                    i32 cz = e->pos.z;
                    if (cz < tile_min_z)
                        cz = tile_min_z;
                    else if (cz > tile_max_z)
                        cz = tile_max_z;

                    i32 diff_x = e->pos.x - cx;
                    i32 diff_z = e->pos.z - cz;

                    i32 dist_sq = fx_mul_q16(diff_x, diff_x) + fx_mul_q16(diff_z, diff_z);
                    i32 r_sq = fx_mul_q16(radius, radius);

                    if (dist_sq < r_sq)
                    {
                        i32 dist = fx_sqrt(dist_sq);
                        if (dist > 100)
                        {
                            i32 overlap = radius - dist;
                            i32 push_x = fx_div_q16(diff_x, dist);
                            i32 push_z = fx_div_q16(diff_z, dist);
                            e->pos.x += fx_mul_q16(push_x, overlap);
                            e->pos.z += fx_mul_q16(push_z, overlap);
                        }
                        else
                        {
                            // Entity center is inside the solid box
                            i32 t_cx = FX_FROM_INT(gx * 2);
                            i32 t_cz = FX_FROM_INT(gz * 2);
                            i32 d_cx = e->pos.x - t_cx;
                            i32 d_cz = e->pos.z - t_cz;
                            if (abs(d_cx) > abs(d_cz))
                            {
                                e->pos.x += (d_cx >= 0) ? (radius + FX_FROM_FLOAT(0.05f)) : -(radius + FX_FROM_FLOAT(0.05f));
                            }
                            else
                            {
                                e->pos.z += (d_cz >= 0) ? (radius + FX_FROM_FLOAT(0.05f)) : -(radius + FX_FROM_FLOAT(0.05f));
                            }
                        }
                    }
                }
            }
        }

        // Check collision against closed doors
        for (int i = 0; i < MAX_ENTITIES_T; i++)
        {
            if (g_entities[i].active && (g_entities[i].type == ENT_TYPE_DOOR || g_entities[i].type == ENT_TYPE_DOOR_LOCKED))
            {
                if (g_entities[i].state == STATE_CLOSED || g_entities[i].state == STATE_CLOSING || g_entities[i].pos.y < FX_FROM_FLOAT(1.4f))
                {
                    Vec4 diff;
                    diff.x = e->pos.x - g_entities[i].pos.x;
                    diff.y = 0;
                    diff.z = e->pos.z - g_entities[i].pos.z;
                    diff.w = 0;
                    i32 dist = vec4_length(&diff);
                    i32 door_radius = FX_FROM_FLOAT(1.1f);
                    if (dist < door_radius && dist > 100)
                    {
                        Vec4 push_dir = diff;
                        vec4_normalize3(&push_dir, &push_dir);
                        i32 overlap = door_radius - dist;
                        e->pos.x += fx_mul_q16(push_dir.x, overlap);
                        e->pos.z += fx_mul_q16(push_dir.z, overlap);
                    }
                }
            }
        }
    }
}

static void fentity_chase_player_astar(FEntity *e)
{
    int idx = e->id;
    if (idx < 0 || idx >= MAX_ENTITIES_T)
        idx = 0;

    int e_x = (int)floorf(FX_TO_FLOAT(e->pos.x) / 2.0f + 0.5f);
    int e_y = (int)floorf(FX_TO_FLOAT(e->pos.z) / 2.0f + 0.5f);
    int p_x = (int)floorf(FX_TO_FLOAT(g_player.w_pos.x) / 2.0f + 0.5f);
    int p_y = (int)floorf(FX_TO_FLOAT(g_player.w_pos.z) / 2.0f + 0.5f);

    i32 dt = g_clock.dt;
    s_path_cooldown[idx] -= dt;

    // Check direct line of sight to player
    int has_los = fai_has_world_los(&e->pos, &g_player.w_pos);

    if (has_los)
    {
        // Direct chase if clear line of sight
        Vec4 dir;
        vec4_sub(&g_player.w_pos, &e->pos, &dir);
        dir.y = 0;
        if (vec4_length(&dir) > FX_FROM_FLOAT(0.05f))
        {
            vec4_normalize3(&dir, &dir);
            float dx = FX_TO_FLOAT(dir.x);
            float dz = FX_TO_FLOAT(dir.z);
            e->rot.y = FX_FROM_FLOAT(atan2f(dz, dx));

            e->pos.x += fx_mul_q16(dir.x, fx_mul_q16(e->speed, dt));
            e->pos.z += fx_mul_q16(dir.z, fx_mul_q16(e->speed, dt));
            fentity_collide_world(e);
        }
        return;
    }

    // Recalculate waypoint if cooldown expired, player moved tile, or reached waypoint
    i32 dist_to_target = vec4_dist(&s_chase_target[idx], &e->pos);
    if (s_path_cooldown[idx] <= 0 || p_x != s_last_px[idx] || p_y != s_last_py[idx] || dist_to_target < FX_FROM_FLOAT(0.35f))
    {
        s_path_cooldown[idx] = FX_FROM_FLOAT(0.2f); // 5Hz path evaluation
        s_last_px[idx] = p_x;
        s_last_py[idx] = p_y;

        Node *goal = fai_find_path(e_x, e_y, p_x, p_y);
        if (goal)
        {
            Node *next = goal;
            // Backtrack to find node immediately following start node
            while (next->parent != NULL && next->parent->parent != NULL)
            {
                next = next->parent;
            }

            s_chase_target[idx].x = FX_FROM_INT(next->x * 2);
            s_chase_target[idx].y = e->pos.y;
            s_chase_target[idx].z = FX_FROM_INT(next->y * 2);
            s_chase_target[idx].w = FX_ONE;
        }
        else
        {
            // Unreachable (blocked by closed door or solid wall): hold position, do not run through walls
            s_chase_target[idx] = e->pos;
        }
    }

    Vec4 dir;
    vec4_sub(&s_chase_target[idx], &e->pos, &dir);
    dir.y = 0;
    if (vec4_length(&dir) > FX_FROM_FLOAT(0.05f))
    {
        vec4_normalize3(&dir, &dir);

        float dx = FX_TO_FLOAT(dir.x);
        float dz = FX_TO_FLOAT(dir.z);
        e->rot.y = FX_FROM_FLOAT(atan2f(dz, dx));

        e->pos.x += fx_mul_q16(dir.x, fx_mul_q16(e->speed, dt));
        e->pos.z += fx_mul_q16(dir.z, fx_mul_q16(e->speed, dt));
        fentity_collide_world(e);
    }
}

void fentity_clear(void)
{
    for (int i = 0; i < MAX_ENTITIES_T; i++)
    {
        g_entities[i].active = 0;
        s_path_cooldown[i] = 0;
        s_last_px[i] = -1;
        s_last_py[i] = -1;
    }
}

static Sprite s_spr_fplsht = {0};
static Sprite s_spr_mplsht = {0};
static Sprite s_spr_unit4 = {0};
static Sprite s_spr_civwrk = {0};
static Sprite s_spr_ledger = {0};
static Sprite s_spr_radio = {0};
static Sprite s_spr_terminal = {0};
static Sprite s_spr_medkit = {0};
static Sprite s_spr_rathole = {0};
static Sprite s_spr_ammo = {0};
static Sprite s_spr_light = {0};

static int s_entity_resources_loaded = 0;

void fentity_init(void)
{
    fentity_clear();

    if (s_entity_resources_loaded)
    {
        return; // Resources already cached; avoid reloading or allocating on the fly
    }

    if (load_sprite(&s_spr_fplsht, (const u8 *)"ASSTS\\TEXTR\\CHAR\\FPLSHT.RAW", 1024, 896))
        console_log("Female police sprites loaded (1024x896)");

    if (load_sprite(&s_spr_mplsht, (const u8 *)"ASSTS\\TEXTR\\CHAR\\MPLSHT.RAW", 1024, 896))
        console_log("Male police sprites loaded (1024x896)");

    if (load_sprite(&s_spr_unit4, (const u8 *)"ASSTS\\TEXTR\\CHAR\\UNIT4.RAW", 1024, 896))
        console_log("UNIT4 sprites loaded (1024x896)");

    if (load_sprite(&s_spr_civwrk, (const u8 *)"ASSTS\\TEXTR\\CHAR\\CIVWRK.RAW", 1024, 896))
        console_log("Civic worker sprites loaded (1024x896)");

    if (load_sprite(&s_spr_ledger, (const u8 *)"ASSTS\\TEXTR\\PROP\\LEDGER.RAW", 128, 128))
        console_log("LEDGER prop loaded (128x128)");

    if (load_sprite(&s_spr_radio, (const u8 *)"ASSTS\\TEXTR\\PROP\\RADIO.RAW", 64, 128))
        console_log("RADIO prop loaded (64x128)");

    if (load_sprite(&s_spr_terminal, (const u8 *)"ASSTS\\TEXTR\\PROP\\TERMINAL.RAW", 64, 128))
        console_log("TERMINAL prop loaded (64x128)");

    if (load_sprite(&s_spr_medkit, (const u8 *)"ASSTS\\TEXTR\\PROP\\MEDKIT.RAW", 128, 64))
        console_log("MEDKIT prop loaded (128x64)");

    if (load_sprite(&s_spr_rathole, (const u8 *)"ASSTS\\TEXTR\\PROP\\RATHOLE.RAW", 64, 64))
        console_log("RATHOLE prop loaded (64x64)");

    if (load_sprite(&s_spr_ammo, (const u8 *)"ASSTS\\TEXTR\\PROP\\AMMO.RAW", 64, 64))
        console_log("AMMO prop loaded (64x64)");

    if (load_sprite(&s_spr_light, (const u8 *)"ASSTS\\TEXTR\\PROP\\LIGHT.RAW", 64, 64))
        console_log("LIGHT prop loaded (64x64)");

    if (load_cms("ASSTS\\MESH\\DOOR\\DOOR.CMS", &g_door_mesh_x))
    {
        load_mesh_textures("ASSTS\\MESH\\DOOR\\DOOR.CMS", &g_door_mesh_x);
    }
    if (load_cms("ASSTS\\MESH\\DOOR\\DOOR.CMS", &g_door_mesh_z))
    {
        load_mesh_textures("ASSTS\\MESH\\DOOR\\DOOR.CMS", &g_door_mesh_z);
        trimes_rotate(&g_door_mesh_z, 0, FX_FROM_FLOAT(1.570796f), 0);
    }

    if (load_cms("ASSTS\\MESH\\DOORL\\DOORL.CMS", &g_doorl_mesh_x))
    {
        load_mesh_textures("ASSTS\\MESH\\DOORL\\DOORL.CMS", &g_doorl_mesh_x);
    }
    if (load_cms("ASSTS\\MESH\\DOORL\\DOORL.CMS", &g_doorl_mesh_z))
    {
        load_mesh_textures("ASSTS\\MESH\\DOORL\\DOORL.CMS", &g_doorl_mesh_z);
        trimes_rotate(&g_doorl_mesh_z, 0, FX_FROM_FLOAT(1.570796f), 0);
    }

    if (load_cms("ASSTS\\MESH\\VAN\\VAN.CMS", &g_van_mesh))
    {
        load_mesh_textures("ASSTS\\MESH\\VAN\\VAN.CMS", &g_van_mesh);
        console_log("VAN 3D mesh and textures loaded (ASSTS\\MESH\\VAN\\VAN.CMS)");
    }

    s_entity_resources_loaded = 1;
}

FEntity *fentity_spawn(FEntityType type, Vec4 pos)
{
    for (int i = 0; i < MAX_ENTITIES_T; i++)
    {
        if (!g_entities[i].active)
        {
            g_entities[i].active = 1;
            g_entities[i].id = i;
            g_entities[i].type = type;
            g_entities[i].state = (type == ENT_TYPE_DOOR || type == ENT_TYPE_DOOR_LOCKED) ? STATE_CLOSED : STATE_IDLE;
            g_entities[i].pos = pos;
            g_entities[i].rot.x = 0;
            g_entities[i].rot.y = 0;
            g_entities[i].rot.z = 0;
            g_entities[i].health = 100;
            g_entities[i].speed = FX_FROM_FLOAT(2.0f);
            g_entities[i].timer = 0;
            return &g_entities[i];
        }
    }
    return NULL;
}

FEntity *fentity_get_all(void)
{
    return g_entities;
}

void fentity_update(void)
{
    // Update AI state machines
    for (int i = 0; i < MAX_ENTITIES_T; i++)
    {
        if (!g_entities[i].active)
            continue;

        FEntity *e = &g_entities[i];
        i32 dist_to_player = vec4_dist(&e->pos, &g_player.w_pos);

        switch (e->type)
        {
        case ENT_TYPE_ENFORCER_F:
        case ENT_TYPE_ENFORCER_M:
            if (e->state == STATE_HIT)
            {
                e->timer -= g_clock.dt;
                if (e->timer <= 0)
                {
                    e->state = STATE_CHASE;
                    e->timer = 0;
                }
            }
            else if (e->state == STATE_IDLE)
            {
                // Look at player distance. If close enough, chase!
                if (dist_to_player < FX_FROM_INT(12))
                {
                    e->state = STATE_CHASE;
                }
            }
            else if (e->state == STATE_CHASE)
            {
                if (dist_to_player > FX_FROM_FLOAT(1.5f))
                {
                    fentity_chase_player_astar(e);
                }
                else
                {
                    // In attack range
                    e->state = STATE_ATTACK;
                    e->timer = FX_FROM_FLOAT(0.25f); // Windup before first strike
                }
            }
            else if (e->state == STATE_ATTACK)
            {
                e->timer += g_clock.dt;
                if (e->timer >= FX_FROM_FLOAT(0.75f))
                {
                    e->timer = 0;
                    g_health -= 10;
                    ui_trigger_damage_flash();
                    if (g_health <= 0)
                    {
                        g_health = 0;
                        g_state = STATE_GAMEOVER;
                        cv_io_mouse_set_mode(CV_MOUSE_MODE_CURSOR);
                        audio_play_sfx(SFX_FEMALE_DEATH);
                    }
                    else
                    {
                        audio_play_sfx(SFX_HIT);
                    }
                }
                if (dist_to_player > FX_FROM_FLOAT(2.0f))
                {
                    e->state = STATE_CHASE;
                    e->timer = 0;
                }
            }
            break;

        case ENT_TYPE_UNIT4:
            // Ally follow behavior
            if (e->state == STATE_IDLE)
            {
                if (g_access_card_given || g_doubt >= 2)
                {
                    e->state = STATE_BETRAYAL;
                    e->timer = FX_FROM_FLOAT(0.25f);
                    if (unit4_betrayal_frame == -1)
                    {
                        unit4_betrayal_frame = g_clock.frame;
                    }
                }
                else if (dist_to_player > FX_FROM_INT(3) && dist_to_player < FX_FROM_INT(14))
                {
                    fentity_chase_player_astar(e);
                }
            }
            else if (e->state == STATE_HIT)
            {
                e->timer -= g_clock.dt;
                if (e->timer <= 0)
                {
                    e->state = (g_access_card_given || g_doubt >= 2) ? STATE_BETRAYAL : STATE_IDLE;
                    e->timer = 0;
                }
            }
            else if (e->state == STATE_BETRAYAL)
            {
                if (dist_to_player > FX_FROM_FLOAT(1.5f))
                {
                    fentity_chase_player_astar(e);
                }
                else
                {
                    e->timer += g_clock.dt;
                    if (e->timer >= FX_FROM_FLOAT(0.75f))
                    {
                        e->timer = 0;
                        g_health -= 10;
                        ui_trigger_damage_flash();
                        if (g_health <= 0)
                        {
                            g_health = 0;
                            g_state = STATE_GAMEOVER;
                            cv_io_mouse_set_mode(CV_MOUSE_MODE_CURSOR);
                            audio_play_sfx(SFX_FEMALE_DEATH);
                        }
                        else
                        {
                            audio_play_sfx(SFX_HIT);
                        }
                    }
                }
            }
            break;

        case ENT_TYPE_BOXER:
            // Passive state. Waits for interaction.
            break;

        case ENT_TYPE_AMMO:
            // Ammo pickup logic
            if (dist_to_player < FX_FROM_FLOAT(1.4f))
            {
                g_ammo += 5;
                if (g_ammo > 25)
                    g_ammo = 25;
                audio_play_sfx(SFX_PICKUP);
                ui_set_subtitle("AMMUNITION ACQUIRED", "+5 Pistol Rounds secured.", "", 240, 43);
                e->active = 0; // Despawn
            }
            break;

        case ENT_TYPE_LIGHT:
        case ENT_TYPE_RADIO:
        case ENT_TYPE_VAN:
        case ENT_TYPE_FACE:
        case ENT_TYPE_EXTRACTION:
        case ENT_TYPE_LEDGER:
        case ENT_TYPE_WORKER:
        case ENT_TYPE_TERMINAL:
            // Static, handled by interactions
            break;

        case ENT_TYPE_DOOR:
        case ENT_TYPE_DOOR_LOCKED:
            if (e->state == STATE_OPENING)
            {
                i32 dt = g_clock.dt;
                i32 door_speed = FX_FROM_FLOAT(2.0f);
                e->pos.y += fx_mul_q16(door_speed, dt);
                if (e->pos.y >= FX_TWO)
                {
                    e->pos.y = FX_TWO;
                    e->state = STATE_OPEN;
                    e->timer = 0;
                    fai_set_obstacle((int)(FX_TO_FLOAT(e->pos.x) / 2.0f + 0.5f),
                                     (int)(FX_TO_FLOAT(e->pos.z) / 2.0f + 0.5f), 0);
                }
            }
            else if (e->state == STATE_OPEN)
            {
                i32 dt = g_clock.dt;
                e->timer += dt;
                if (e->timer >= FX_FROM_FLOAT(3.5f))
                {
                    // Check for obstruction (player or other entities)
                    int blocked = 0;
                    Vec4 p_diff;
                    p_diff.x = g_player.w_pos.x - e->pos.x;
                    p_diff.y = 0;
                    p_diff.z = g_player.w_pos.z - e->pos.z;
                    p_diff.w = 0;
                    if (vec4_length(&p_diff) < FX_FROM_FLOAT(1.5f))
                    {
                        blocked = 1;
                    }

                    if (!blocked)
                    {
                        for (int j = 0; j < MAX_ENTITIES_T; j++)
                        {
                            if (i == j || !g_entities[j].active)
                                continue;
                            if (g_entities[j].type == ENT_TYPE_DOOR || g_entities[j].type == ENT_TYPE_DOOR_LOCKED)
                                continue;
                            Vec4 ediff;
                            ediff.x = g_entities[j].pos.x - e->pos.x;
                            ediff.y = 0;
                            ediff.z = g_entities[j].pos.z - e->pos.z;
                            ediff.w = 0;
                            if (vec4_length(&ediff) < FX_FROM_FLOAT(1.2f))
                            {
                                blocked = 1;
                                break;
                            }
                        }
                    }

                    if (blocked)
                    {
                        e->timer = FX_FROM_FLOAT(2.5f); // Hold open and re-check shortly
                    }
                    else
                    {
                        e->state = STATE_CLOSING;
                        audio_play_sfx_at(SFX_DOOR_OPEN, &e->pos);
                    }
                }
            }
            else if (e->state == STATE_CLOSING)
            {
                int blocked = 0;
                Vec4 p_diff;
                p_diff.x = g_player.w_pos.x - e->pos.x;
                p_diff.y = 0;
                p_diff.z = g_player.w_pos.z - e->pos.z;
                p_diff.w = 0;
                if (vec4_length(&p_diff) < FX_FROM_FLOAT(1.5f))
                {
                    blocked = 1;
                }

                if (!blocked)
                {
                    for (int j = 0; j < MAX_ENTITIES_T; j++)
                    {
                        if (i == j || !g_entities[j].active)
                            continue;
                        if (g_entities[j].type == ENT_TYPE_DOOR || g_entities[j].type == ENT_TYPE_DOOR_LOCKED)
                            continue;
                        Vec4 ediff;
                        ediff.x = g_entities[j].pos.x - e->pos.x;
                        ediff.y = 0;
                        ediff.z = g_entities[j].pos.z - e->pos.z;
                        ediff.w = 0;
                        if (vec4_length(&ediff) < FX_FROM_FLOAT(1.2f))
                        {
                            blocked = 1;
                            break;
                        }
                    }
                }

                if (blocked)
                {
                    // Re-open if blocked while closing
                    e->state = STATE_OPENING;
                    audio_play_sfx_at(SFX_DOOR_OPEN, &e->pos);
                }
                else
                {
                    i32 dt = g_clock.dt;
                    i32 door_speed = FX_FROM_FLOAT(2.0f);
                    e->pos.y -= fx_mul_q16(door_speed, dt);
                    if (e->pos.y <= 0)
                    {
                        e->pos.y = 0;
                        e->state = STATE_CLOSED;
                        fai_set_obstacle((int)(FX_TO_FLOAT(e->pos.x) / 2.0f + 0.5f),
                                         (int)(FX_TO_FLOAT(e->pos.z) / 2.0f + 0.5f), 1);
                    }
                }
            }
            break;

        case ENT_TYPE_MEDKIT:
            // Medkit trap logic
            if (dist_to_player < FX_FROM_FLOAT(1.5f))
            {
                int is_warm = flevel_is_in_warm_light(g_player.w_pos);
                if (is_warm && g_loyalty >= 2)
                {
                    // It's a trap, spawn an enforcer in chase state
                    FEntity *ambush = fentity_spawn(ENT_TYPE_ENFORCER_F, e->pos);
                    if (ambush)
                    {
                        ambush->state = STATE_CHASE;
                    }
                    audio_play_sfx(SFX_ALARM);
                    ui_set_subtitle("DIRECTORATE TRAP", "AMBUSH: Loyalty test triggered enforcer counter-measure!", "", 360, 40);
                }
                else
                {
                    // Normal medkit
                    g_health += 25;
                    if (g_health > 100)
                        g_health = 100;
                    audio_play_sfx(SFX_PICKUP);
                }
                e->active = 0; // Despawn
            }
            break;
        }
    }
}

Sprite *fentity_get_sprite(FEntityType type)
{
    switch (type)
    {
    case ENT_TYPE_ENFORCER_F:
        return s_spr_fplsht.pix ? &s_spr_fplsht : NULL;
    case ENT_TYPE_ENFORCER_M:
    case ENT_TYPE_BOXER:
        return s_spr_mplsht.pix ? &s_spr_mplsht : NULL;
    case ENT_TYPE_WORKER:
        return s_spr_civwrk.pix ? &s_spr_civwrk : NULL;
    case ENT_TYPE_UNIT4:
        return s_spr_unit4.pix ? &s_spr_unit4 : NULL;
    case ENT_TYPE_LEDGER:
        return s_spr_ledger.pix ? &s_spr_ledger : NULL;
    case ENT_TYPE_RADIO:
        return s_spr_radio.pix ? &s_spr_radio : NULL;
    case ENT_TYPE_TERMINAL:
        return s_spr_terminal.pix ? &s_spr_terminal : NULL;
    case ENT_TYPE_MEDKIT:
        return s_spr_medkit.pix ? &s_spr_medkit : NULL;
    case ENT_TYPE_AMMO:
        return s_spr_ammo.pix ? &s_spr_ammo : NULL;
    case ENT_TYPE_LIGHT:
        return s_spr_light.pix ? &s_spr_light : NULL;
    case ENT_TYPE_RATHOLE:
        return s_spr_rathole.pix ? &s_spr_rathole : NULL;
    default:
        return NULL;
    }
}

static void get_enforcer_uv(FEntity *e, const Camera *cam, int *su, int *sv, int *sw, int *sh)
{
    float dx = FX_TO_FLOAT(cam->position.x - e->pos.x);
    float dz = FX_TO_FLOAT(cam->position.z - e->pos.z);
    float angle_to_cam = atan2f(dz, dx);
    float facing = FX_TO_FLOAT(e->rot.y);
    float relative = facing - angle_to_cam;
    float two_pi = 6.28318530718f;

    while (relative < 0.0f)
        relative += two_pi;
    while (relative >= two_pi)
        relative -= two_pi;

    int angle_idx = (int)((relative + (3.14159265f / 8.0f)) / (3.14159265f / 4.0f)) % 8;
    if (angle_idx < 0)
        angle_idx += 8;

    int frame_row = 0;

    if (e->state == STATE_IDLE)
    {
        frame_row = 0;
    }
    else if (e->state == STATE_CHASE)
    {
        int anim_frame = (g_clock.frame / 20) % 4;
        frame_row = anim_frame;
    }
    else if (e->state == STATE_ATTACK)
    {
        int anim_frame = (g_clock.frame / 40) % 2;
        frame_row = 4 + anim_frame;
    }
    else if (e->state == STATE_HIT)
    {
        frame_row = 6; // Valid 7th row in 896px sheet (0..6)
    }
    else
    {
        frame_row = 0;
    }

    *su = angle_idx * 128;
    *sv = frame_row * 128;
    *sw = 128;
    *sh = 128;
}

void fentity_draw(Camera *cam, Surface8 *surf, const ClipRect *clip_rect)
{
    FEntitySortNode sort_nodes[MAX_ENTITIES_T];
    int sort_count = 0;

    for (int i = 0; i < MAX_ENTITIES_T; i++)
    {
        if (!g_entities[i].active)
            continue;

        sort_nodes[sort_count].e = &g_entities[i];
        sort_nodes[sort_count].dist = vec4_dist(&g_entities[i].pos, &cam->position);
        sort_count++;
    }

    qsort(sort_nodes, sort_count, sizeof(FEntitySortNode), compare_entity_dist);

    // Render entities as billboards in CyberVGA
    for (int i = 0; i < sort_count; i++)
    {
        FEntity *e = sort_nodes[i].e;

        if (e->type == ENT_TYPE_DOOR || e->type == ENT_TYPE_DOOR_LOCKED)
        {
            // Door center in world coordinates (mid-height of door slab)
            Vec4 door_center = e->pos;
            door_center.y += FX_FROM_FLOAT(1.0f);
            door_center.w = FX_ONE;

            // Transform door center into camera space
            Vec4 cpos;
            cam_world_to_cam(cam, &door_center, &cpos);

            // Door bounding radius ~1.4 units
            const i32 door_radius = FX_FROM_FLOAT(1.4f);

            //  Cull if completely behind camera near plane
            if (cpos.z + door_radius < cam->z_near)
            {
                continue;
            }

            //  Cull if door center is behind camera eye and outside radius
            if (cpos.z < 0 && vec4_dist(&door_center, &cam->position) > door_radius)
            {
                continue;
            }

            // Cull if beyond camera far plane
            if (cpos.z - door_radius > cam->z_far)
            {
                continue;
            }

            // Frustum culling in X and Y
            if (cpos.z > 0 && cam->fov_slope_x > 0 && cam->fov_slope_y > 0)
            {
                i32 max_x = fx_mul_q16(cpos.z, cam->fov_slope_x) + door_radius;
                i32 max_y = fx_mul_q16(cpos.z, cam->fov_slope_y) + door_radius;
                if (cpos.x < -max_x || cpos.x > max_x || cpos.y < -max_y || cpos.y > max_y)
                {
                    continue;
                }
            }

            MeshCMS *m = NULL;
            if (e->type == ENT_TYPE_DOOR_LOCKED)
            {
                m = (e->rot.y != 0) ? &g_doorl_mesh_z : &g_doorl_mesh_x;
            }
            else
            {
                m = (e->rot.y != 0) ? &g_door_mesh_z : &g_door_mesh_x;
            }

            if (m && m->vertex_count > 0)
            {
                ClipSpans spans;
                clipspans_from_cliprect(clip_rect, &spans);
                draw_trimes_tex(m, cam, surf, &e->pos, g_world.num_pointLights, g_world.pointLights, NULL, &spans);
            }
            continue;
        }

        if (e->type == ENT_TYPE_VAN)
        {
            Vec4 van_center = e->pos;
            van_center.y = 0;
            van_center.w = FX_ONE;

            Vec4 cpos;
            cam_world_to_cam(cam, &van_center, &cpos);

            const i32 van_radius = FX_FROM_FLOAT(3.5f);

            // Cull if completely behind camera near plane
            if (cpos.z + van_radius < cam->z_near)
            {
                continue;
            }

            //  Cull if center is behind camera eye and outside radius
            if (cpos.z < 0 && vec4_dist(&van_center, &cam->position) > van_radius)
            {
                continue;
            }

            //  Cull if beyond camera far plane
            if (cpos.z - van_radius > cam->z_far)
            {
                continue;
            }

            // Frustum culling in X and Y
            if (cpos.z > 0 && cam->fov_slope_x > 0 && cam->fov_slope_y > 0)
            {
                i32 max_x = fx_mul_q16(cpos.z, cam->fov_slope_x) + van_radius;
                i32 max_y = fx_mul_q16(cpos.z, cam->fov_slope_y) + van_radius;
                if (cpos.x < -max_x || cpos.x > max_x || cpos.y < -max_y || cpos.y > max_y)
                {
                    continue;
                }
            }

            if (g_van_mesh.vertex_count > 0)
            {
                ClipSpans spans;
                clipspans_from_cliprect(clip_rect, &spans);
                draw_trimes_tex(&g_van_mesh, cam, surf, &e->pos, g_world.num_pointLights, g_world.pointLights, NULL, &spans);
            }
            continue;
        }

        Sprite *spr = fentity_get_sprite(e->type);

        if (spr && spr->pix)
        {
            // Assume entities are roughly 1x2 or 1x1 units in world space
            i32 w = FX_ONE;
            i32 h = (e->type == ENT_TYPE_RADIO) ? FX_FROM_FLOAT(0.5f) : FX_ONE;

            if (e->type == ENT_TYPE_ENFORCER_F || e->type == ENT_TYPE_ENFORCER_M ||
                e->type == ENT_TYPE_UNIT4 || e->type == ENT_TYPE_BOXER || e->type == ENT_TYPE_WORKER)
            {
                int su, sv, sw, sh;
                get_enforcer_uv(e, cam, &su, &sv, &sw, &sh);
                w = h = FX_ONE + (65536 / 3);
                draw_sprite_billboard_sub(
                    surf->back,
                    &e->pos,
                    w, h,
                    cam,
                    spr,
                    su, sv, sw, sh,
                    255, // color_key
                    g_world.num_pointLights,
                    g_world.pointLights,
                    clip_rect);
            }
            else
            {
                Vec4 render_pos = e->pos;

                if (e->type == ENT_TYPE_RADIO)
                {
                    w = FX_FROM_FLOAT(0.75f);
                    h = FX_FROM_FLOAT(1.5f);
                }
                else if (e->type == ENT_TYPE_TERMINAL)
                {
                    w = FX_FROM_FLOAT(0.75f);
                    h = FX_FROM_FLOAT(1.5f);
                }
                else if (e->type == ENT_TYPE_MEDKIT)
                {
                    w = FX_FROM_FLOAT(0.75f);
                    h = FX_FROM_FLOAT(0.75f);
                    render_pos.y = FX_FROM_FLOAT(0.35f);
                }
                else if (e->type == ENT_TYPE_LEDGER)
                {
                    w = FX_FROM_FLOAT(0.75f);
                    h = FX_FROM_FLOAT(0.75f);
                    render_pos.y = FX_FROM_FLOAT(0.45f);
                }
                else if (e->type == ENT_TYPE_RATHOLE)
                {
                    w = FX_FROM_FLOAT(0.75f);
                    h = FX_FROM_FLOAT(0.75f);
                    render_pos.y = FX_FROM_FLOAT(0.45f);
                }
                else if (e->type == ENT_TYPE_AMMO)
                {
                    w = FX_FROM_FLOAT(0.75f);
                    h = FX_FROM_FLOAT(0.38f);
                    render_pos.y = FX_FROM_FLOAT(0.20f);
                }
                else if (e->type == ENT_TYPE_LIGHT)
                {
                    w = FX_FROM_FLOAT(0.75f);
                    h = FX_FROM_FLOAT(0.75f);
                    render_pos.y = FX_FROM_FLOAT(2.0f) - FX_FROM_FLOAT(0.375f);
                }

                draw_sprite_billboard(
                    surf->back,
                    &render_pos,
                    w, h,
                    cam,
                    spr,
                    255, // color_key = 255
                    g_world.num_pointLights,
                    g_world.pointLights,
                    clip_rect);
            }
        }
    }
}
