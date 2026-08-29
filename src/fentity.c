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

#define MAX_ENTITIES_T 128

typedef struct {
    FEntity *e;
    i32 dist;
} FEntitySortNode;

static int compare_entity_dist(const void *a, const void *b)
{
    FEntitySortNode *na = (FEntitySortNode *)a;
    FEntitySortNode *nb = (FEntitySortNode *)b;
    if (na->dist < nb->dist) return 1;
    if (na->dist > nb->dist) return -1;
    return 0;
}

static FEntity g_entities[MAX_ENTITIES_T];
static MeshCMS g_door_mesh_x;
static MeshCMS g_door_mesh_z;
static MeshCMS g_doorl_mesh_x;
static MeshCMS g_doorl_mesh_z;

static void fentity_chase_player_astar(FEntity *e)
{
    int e_x = (int)(FX_TO_FLOAT(e->pos.x) / 2.0f + 0.5f);
    int e_y = (int)(FX_TO_FLOAT(e->pos.z) / 2.0f + 0.5f);
    int p_x = (int)(FX_TO_FLOAT(g_player.w_pos.x) / 2.0f + 0.5f);
    int p_y = (int)(FX_TO_FLOAT(g_player.w_pos.z) / 2.0f + 0.5f);

    Node *goal = fai_find_path(e_x, e_y, p_x, p_y);
    if (goal)
    {
        Node *next = goal;
        // Backtrack to find the node immediately following the start node
        while (next->parent != NULL && next->parent->parent != NULL)
        {
            next = next->parent;
        }

        Vec4 target;
        target.x = FX_FROM_INT(next->x * 2);
        target.y = e->pos.y;
        target.z = FX_FROM_INT(next->y * 2);
        target.w = FX_ONE;

        Vec4 dir;
        vec4_sub(&target, &e->pos, &dir);

        // Prevent moving completely erratic when on top of the tile center
        i32 dist_to_target = vec4_dist(&target, &e->pos);
        if (dist_to_target > FX_FROM_FLOAT(0.01f))
        {
            vec4_normalize3(&dir, &dir);

            float dx = FX_TO_FLOAT(dir.x);
            float dz = FX_TO_FLOAT(dir.z);
            e->rot.y = FX_FROM_FLOAT(atan2f(dz, dx));

            i32 dt = g_clock.dt;
            e->pos.x += fx_mul_q16(dir.x, fx_mul_q16(e->speed, dt));
            e->pos.z += fx_mul_q16(dir.z, fx_mul_q16(e->speed, dt));
        }
    }
    else
    {
        if (g_clock.frame % 60 == 0)
        {
            console_log("No path found for entity ID %d\n", e->id);
        }
    }
}

void fentity_init(void)
{
    for (int i = 0; i < MAX_ENTITIES_T; i++)
    {
        g_entities[i].active = 0;
    }

    int slot = -1;
    for (int j = 0; j < MAX_TEXTURES; j++)
    {
        if (!m_textures[j].pix)
        {
            slot = j;
            break;
        }
    }
    if (slot >= 0)
    {
        if (load_sprite(&m_textures[slot], (const u8 *)"ASSTS\\TEXTR\\CHAR\\FPLSHT.RAW", 1024, 896))
        {
            m_textures[slot].name = (u8 *)strdup("FPLSHT.RAW");
            m_textures[slot].path = strdup("FPLSHT.RAW");
        }
    }

    int mslot = -1;
    for (int j = 0; j < MAX_TEXTURES; j++)
    {
        if (!m_textures[j].pix)
        {
            mslot = j;
            break;
        }
    }
    if (mslot >= 0)
    {
        if (load_sprite(&m_textures[mslot], (const u8 *)"ASSTS\\TEXTR\\CHAR\\MPLSHT.RAW", 1024, 896))
        {
            m_textures[mslot].name = (u8 *)strdup("MPLSHT.RAW");
            m_textures[mslot].path = strdup("MPLSHT.RAW");
        }
    }

    if (load_cms("ASSTS\\MESH\\DOOR\\DOOR.CMS", &g_door_mesh_x))
    {
        load_mesh_textures("ASSTS\\MESH\\DOOR\\DOOR.CMS", &g_door_mesh_x);
    }
    if (load_cms("ASSTS\\MESH\\DOOR\\DOOR.CMS", &g_door_mesh_z))
    {
        load_mesh_textures("ASSTS\\MESH\\DOOR\\DOOR.CMS", &g_door_mesh_z);
        trimes_rotate(&g_door_mesh_z, 0, FX_FROM_FLOAT(1.570796f), 0);
    }

    if (load_cms("ASSTS\\MESH\\DOOR\\DOORL.CMS", &g_doorl_mesh_x))
    {
        load_mesh_textures("ASSTS\\MESH\\DOOR\\DOORL.CMS", &g_doorl_mesh_x);
    }
    if (load_cms("ASSTS\\MESH\\DOOR\\DOORL.CMS", &g_doorl_mesh_z))
    {
        load_mesh_textures("ASSTS\\MESH\\DOOR\\DOORL.CMS", &g_doorl_mesh_z);
        trimes_rotate(&g_doorl_mesh_z, 0, FX_FROM_FLOAT(1.570796f), 0);
    }
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
            if (e->state == STATE_IDLE)
            {
                // Look at player distance. If close enough, chase!
                if (dist_to_player < FX_FROM_INT(8))
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
                }
            }
            else if (e->state == STATE_ATTACK)
            {
                // Attack logic here (reduce g_health)
                if (g_clock.frame % 30 == 0)
                {
                    g_health -= 5;
                }
                if (dist_to_player > FX_FROM_FLOAT(2.0f))
                {
                    e->state = STATE_CHASE; // Player ran away, chase again
                }
            }
            break;

        case ENT_TYPE_UNIT4:
            // Ally follow behavior
            if (e->state == STATE_IDLE)
            {
                // Trigger betrayal if doubt is high enough
                if (g_doubt >= 2)
                {
                    e->state = STATE_BETRAYAL;
                }
                // Follow player if further than 3 units
                else if (dist_to_player > FX_FROM_INT(3) && dist_to_player < FX_FROM_INT(12))
                {
                    Vec4 dir;
                    vec4_sub(&g_player.w_pos, &e->pos, &dir);
                    vec4_normalize3(&dir, &dir);

                    i32 dt = g_clock.dt;
                    e->pos.x += fx_mul_q16(dir.x, fx_mul_q16(e->speed, dt));
                    e->pos.z += fx_mul_q16(dir.z, fx_mul_q16(e->speed, dt));
                }
            }
            else if (e->state == STATE_BETRAYAL)
            {
                // Behaves exactly like an Enforcer now
                if (dist_to_player > FX_FROM_FLOAT(1.5f))
                {
                    fentity_chase_player_astar(e);
                }
                else
                {
                    if (g_clock.frame % 30 == 0)
                    {
                        g_health -= 5;
                    }
                }
            }
            break;

        case ENT_TYPE_BOXER:
            // Passive state. Waits for interaction.
            break;

        case ENT_TYPE_RADIO:
        case ENT_TYPE_VAN:
        case ENT_TYPE_FACE:
        case ENT_TYPE_EXTRACTION:
            // Static, handled by interactions
            break;

        case ENT_TYPE_MEDKIT:
            // Medkit trap logic
            if (dist_to_player < FX_FROM_FLOAT(1.5f))
            {
                if (g_loyalty >= 2)
                {
                    // It's a trap, spawn an enforcer
                    fentity_spawn(ENT_TYPE_ENFORCER_F, e->pos);
                }
                else
                {
                    // Normal medkit
                    g_health += 25;
                    if (g_health > 100)
                        g_health = 100;
                }
                e->active = 0; // Despawn
            }
            break;

        case ENT_TYPE_PROJECTILE:
            // Move projectile forward
            {
                i32 dt = g_clock.dt;
                // Use rot as direction vector (already normalized when spawned)
                e->pos.x += fx_mul_q16(e->rot.x, fx_mul_q16(e->speed, dt));
                e->pos.y += fx_mul_q16(e->rot.y, fx_mul_q16(e->speed, dt));
                e->pos.z += fx_mul_q16(e->rot.z, fx_mul_q16(e->speed, dt));

                e->timer += dt;
                if (e->timer > FX_FROM_INT(2)) // 2 seconds lifetime
                {
                    e->active = 0;
                }
                else
                {
                    // Check collision with enemies
                    for (int j = 0; j < MAX_ENTITIES_T; j++)
                    {
                        if (i == j || !g_entities[j].active)
                            continue;
                        FEntity *other = &g_entities[j];

                        if (other->type == ENT_TYPE_ENFORCER_F || other->type == ENT_TYPE_UNIT4 || other->type == ENT_TYPE_BOXER)
                        {
                            i32 dist = vec4_dist(&e->pos, &other->pos);
                            if (dist < FX_ONE)
                            {
                                other->health -= 34; // 3 shots to kill
                                if (other->health <= 0)
                                {
                                    other->active = 0;
                                    if (other->type == ENT_TYPE_BOXER)
                                        g_boxer_dead = 1;
                                }
                                e->active = 0;
                                break;
                            }
                        }
                    }
                }
            }
            break;
        }
    }
}

static Sprite *get_sprite_for_entity(FEntityType type)
{
    const char *tex_name = 0x0;
    switch (type)
    {
    case ENT_TYPE_ENFORCER_F:
        tex_name = "FPLSHT.RAW";
        break;
    case ENT_TYPE_ENFORCER_M:
        tex_name = "MPLSHT.RAW";
        break;
    case ENT_TYPE_UNIT4:
        tex_name = "UNIT4";
        break;
    case ENT_TYPE_BOXER:
        tex_name = "BOXER";
        break;
    case ENT_TYPE_RADIO:
        tex_name = "RADIO";
        break;
    case ENT_TYPE_MEDKIT:
        tex_name = "MEDKIT";
        break;
    case ENT_TYPE_VAN:
        tex_name = "VAN";
        break;
    case ENT_TYPE_FACE:
        tex_name = "FACE";
        break;
    default:
        return 0x0;
    }

    int idx = textures_find_by_name(m_textures, MAX_TEXTURES, tex_name);
    if (idx >= 0)
    {
        return &m_textures[idx];
    }
    return 0x0;
}

static void get_enforcer_uv(FEntity *e, const Camera *cam, int *su, int *sv, int *sw, int *sh)
{
    float dx = FX_TO_FLOAT(cam->position.x - e->pos.x);
    float dz = FX_TO_FLOAT(cam->position.z - e->pos.z);
    float angle_to_cam = atan2f(dz, dx);
    float facing = FX_TO_FLOAT(e->rot.y);
    float relative = facing - angle_to_cam;
    float PI2 = 6.28318530718f;

    while (relative < 0.0f)
        relative += PI2;
    while (relative >= PI2)
        relative -= PI2;

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
        int anim_frame = (g_clock.frame / 10) % 4;
        frame_row = anim_frame;
    }
    else if (e->state == STATE_ATTACK)
    {
        int anim_frame = (g_clock.frame / 5) % 2;
        frame_row = 4 + anim_frame;
    }
    else if (e->state == STATE_HIT)
    {
        int anim_frame = (e->timer / 5) % 2; // Hit 1 or 2
        frame_row = 6 + anim_frame;
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
            if (e->state == STATE_CLOSED)
            {
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
            }
            continue;
        }

        Sprite *spr = get_sprite_for_entity(e->type);

        if (spr && spr->pix)
        {
            // Assume entities are roughly 1x2 or 1x1 units in world space
            i32 w = FX_ONE;
            i32 h = (e->type == ENT_TYPE_RADIO) ? FX_FROM_FLOAT(0.5f) : FX_ONE;

            if (e->type == ENT_TYPE_ENFORCER_F || e->type == ENT_TYPE_ENFORCER_M)
            {
                int su, sv, sw, sh;
                get_enforcer_uv(e, cam, &su, &sv, &sw, &sh);
                w = h = FX_TWO;
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
                draw_sprite_billboard(
                    surf->back,
                    &e->pos,
                    w, h,
                    cam,
                    spr,
                    255, // color_key = 255 usually
                    g_world.num_pointLights,
                    g_world.pointLights,
                    clip_rect);
            }
        }
    }
}
