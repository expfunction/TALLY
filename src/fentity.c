#include "fentity.h"
#include "fgame.h"
#include "fplayr.h"
#include "CORE/MTEXTUR.H"
#include "RNDR/SPRIT.H"

#define MAX_ENTITIES_T 128

static FEntity g_entities[MAX_ENTITIES_T];

void fentity_init(void)
{
    for (int i = 0; i < MAX_ENTITIES_T; i++)
    {
        g_entities[i].active = 0;
    }
}

void fentity_spawn(FEntityType type, Vec4 pos)
{
    for (int i = 0; i < MAX_ENTITIES_T; i++)
    {
        if (!g_entities[i].active)
        {
            g_entities[i].active = 1;
            g_entities[i].id = i;
            g_entities[i].type = type;
            g_entities[i].state = STATE_IDLE;
            g_entities[i].pos = pos;
            g_entities[i].health = 100;
            g_entities[i].speed = FX_FROM_FLOAT(2.0f);
            g_entities[i].timer = 0;
            break;
        }
    }
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
        case ENT_TYPE_ENFORCER:
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
                    // Move towards player
                    Vec4 dir;
                    vec4_sub(&g_player.w_pos, &e->pos, &dir);
                    vec4_normalize3(&dir, &dir);

                    i32 dt = g_clock.dt;
                    e->pos.x += fx_mul_q16(dir.x, fx_mul_q16(e->speed, dt));
                    e->pos.z += fx_mul_q16(dir.z, fx_mul_q16(e->speed, dt));
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
                    // g_health -= 5;
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
                // Follow player if further than 3 units
                if (dist_to_player > FX_FROM_INT(3) && dist_to_player < FX_FROM_INT(12))
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
                    Vec4 dir;
                    vec4_sub(&g_player.w_pos, &e->pos, &dir);
                    vec4_normalize3(&dir, &dir);

                    i32 dt = g_clock.dt;
                    e->pos.x += fx_mul_q16(dir.x, fx_mul_q16(e->speed, dt));
                    e->pos.z += fx_mul_q16(dir.z, fx_mul_q16(e->speed, dt));
                }
            }
            break;

        case ENT_TYPE_BOXER:
            // Passive state. Waits for interaction.
            break;

        case ENT_TYPE_RADIO:
            // Static
            break;
        }
    }
}

static Sprite *get_sprite_for_entity(FEntityType type)
{
    const char *tex_name = 0x0;
    switch (type)
    {
    case ENT_TYPE_ENFORCER:
        tex_name = "ENFORCER";
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

void fentity_draw(Camera *cam, Surface8 *surf, const ClipRect *clip_rect)
{
    // Render entities as billboards in CyberVGA
    for (int i = 0; i < MAX_ENTITIES_T; i++)
    {
        if (!g_entities[i].active)
            continue;

        FEntity *e = &g_entities[i];
        Sprite *spr = get_sprite_for_entity(e->type);

        if (spr && spr->pix)
        {
            // Assume entities are roughly 1x2 or 1x1 units in world space
            i32 w = FX_FROM_FLOAT(1.0f);
            i32 h = (e->type == ENT_TYPE_RADIO) ? FX_FROM_FLOAT(0.5f) : FX_FROM_FLOAT(2.0f);

            draw_sprite_billboard(
                surf->back,
                &e->pos,
                w, h,
                cam,
                spr,
                255, // color_key = magenta usually, assume 255
                g_world.num_pointLights,
                g_world.pointLights,
                clip_rect // clip_rect (we'd need to pass the real clip rect if we wanted to clip to portals)
            );
        }
    }
}
