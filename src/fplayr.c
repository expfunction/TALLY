/* Copyright (c) 2026 Burak Yazar */

#include "fplayr.h"
#include "IO/IO.H"
#include "RNDR/CAMER.H"
#include "PHYS/PHYS.H"
#include "CORE/CYBER.H"

FPlayer g_player;

void fplayer_init(void)
{
    g_player.w_pos.x = 0;
    g_player.w_pos.y = FX_FROM_FLOAT(0.5f); // Start a bit above ground
    g_player.w_pos.z = 0;
    g_player.w_pos.w = FX_ONE;
    g_player.w_rot.x = 0; // pitch
    g_player.w_rot.y = 0; // yaw
}

void fplayer_set_start_pos(Vec4 pos)
{
    g_player.w_pos = pos;
    g_player.w_pos.y = FX_FROM_FLOAT(0.5f);
}

void fplayer_update(Camera *cam)
{
    // 1. Mouse look
    MouseIO mouse;
    if (cv_io_mouse_state(&mouse))
    {
        // dx and dy are integer screen deltas
        g_player.w_rot.y += FX_FROM_INT(mouse.dx) / 100; // Yaw (Reversed)
        g_player.w_rot.x += FX_FROM_INT(mouse.dy) / 100; // Pitch

        // Clamp pitch to avoid flipping
        i32 pitch_limit = FX_FROM_FLOAT(1.5f); // ~85 degrees
        if (g_player.w_rot.x > pitch_limit)
            g_player.w_rot.x = pitch_limit;
        if (g_player.w_rot.x < -pitch_limit)
            g_player.w_rot.x = -pitch_limit;
    }

    // 2. WASD Movement based on yaw
    i32 move_speed = fx_mul_q16(FX_FROM_FLOAT(4.0f), g_clock.dt); // 4 units per second
    i32 dx = 0;
    i32 dz = 0;

    if (cv_io_key_down(KEY_W))
        dz += move_speed;
    if (cv_io_key_down(KEY_S))
        dz -= move_speed;
    if (cv_io_key_down(KEY_A))
        dx -= move_speed;
    if (cv_io_key_down(KEY_D))
        dx += move_speed;

    // Apply movement relative to yaw
    if (dx != 0 || dz != 0)
    {
        i32 sin_y = fx_sin(g_player.w_rot.y);
        i32 cos_y = fx_cos(g_player.w_rot.y);

        // Forward/back
        g_player.w_pos.x += fx_mul_q16(dz, sin_y);
        g_player.w_pos.z += fx_mul_q16(dz, cos_y);

        // Strafe
        g_player.w_pos.x += fx_mul_q16(dx, cos_y);
        g_player.w_pos.z -= fx_mul_q16(dx, sin_y);
    }

    // Hook camera
    Vec3 p = {g_player.w_pos.x, g_player.w_pos.y, g_player.w_pos.z};
    Vec3 r = {g_player.w_rot.x, g_player.w_rot.y, 0};
    cam_init(cam, &p, &r, CAM_PROJ_PERSPECTIVE);

    // Optional: Collide camera against the world walls using physics system
    phys_collide_camera(cam, &g_world);
    g_player.w_pos = cam->position; // Update back player position if pushed

    // Interaction check
    if (cv_io_key_pressed_now(KEY_E))
    {
        // Simple raycast for interaction
        FEntity *ents = fentity_get_all();
        for (int i = 0; i < 128; i++)
        {
            if (!ents[i].active)
                continue;

            i32 dist = vec4_dist(&g_player.w_pos, &ents[i].pos);
            if (dist < FX_FROM_FLOAT(2.5f))
            {
                if (ents[i].type == ENT_TYPE_EXTRACTION)
                {
                    if (g_current_level == LEVEL_BARRACKS)
                    {
                        if (g_mission_progress == 1)
                            fgame_load_level(LEVEL_RATIONBLOCK);
                        else if (g_mission_progress == 2)
                            fgame_load_level(LEVEL_GENERATOR);
                        else
                            g_state = STATE_WIN; // After endings
                    }
                    else
                    {
                        g_mission_progress++;
                        fgame_load_level(LEVEL_BARRACKS);
                    }
                    // Prevent multiple triggers
                    g_player.w_pos.x += FX_FROM_FLOAT(5.0f);
                    break;
                }
                else if (ents[i].type == ENT_TYPE_RADIO)
                {
                    // Interact with radio
                    // Modify loyalty/doubt based on lighting (simplified here)
                    g_loyalty += 1;
                    break;
                }
                else if (ents[i].type == ENT_TYPE_BOXER)
                {
                    // Boxer ending triggers
                    if (!g_boxer_dead)
                    {
                        g_doubt += 1; // Read the law
                        // If doubt >= 3, ending B can happen at rathole
                    }
                    break;
                }
                else if (ents[i].type == ENT_TYPE_VAN)
                {
                    // Van logic (Ending A or C)
                    if (g_boxer_dead)
                    {
                        if (g_loyalty >= 3 && g_doubt >= 2)
                        {
                            g_ending = 3; // C (Promotion)
                        }
                        else
                        {
                            g_ending = 1; // A (Reliable)
                        }
                        g_state = STATE_WIN;
                    }
                    break;
                }
                else if (ents[i].type == ENT_TYPE_RATHOLE)
                {
                    // Ending B (Refusal)
                    if (g_doubt >= 3)
                    {
                        g_ending = 2; // B
                        g_state = STATE_WIN;
                    }
                    break;
                }
            }
        }
    }

    // Shooting mechanics (LMB)
    static int prev_lmb = 0;
    if (mouse.btn_left && !prev_lmb)
    {
        // Shoot projectile
        if (g_ammo > 0)
        {
            g_ammo--;
            // Direction is based on yaw/pitch
            Vec4 p_pos = g_player.w_pos;
            p_pos.y -= FX_FROM_FLOAT(0.1f); // slightly below eye height

            fentity_spawn(ENT_TYPE_PROJECTILE, p_pos);

            // Set projectile direction and speed
            FEntity *ents = fentity_get_all();
            for (int i = 0; i < 128; i++)
            {
                if (ents[i].active && ents[i].type == ENT_TYPE_PROJECTILE && ents[i].timer == 0)
                {
                    // Newly spawned projectile
                    i32 s_y = fx_sin(g_player.w_rot.y);
                    i32 c_y = fx_cos(g_player.w_rot.y);
                    i32 s_x = fx_sin(g_player.w_rot.x);
                    i32 c_x = fx_cos(g_player.w_rot.x);

                    // Forward vector
                    ents[i].rot.x = fx_mul_q16(c_x, s_y);
                    ents[i].rot.y = -s_x;
                    ents[i].rot.z = fx_mul_q16(c_x, c_y);
                    ents[i].speed = FX_FROM_FLOAT(15.0f); // Fast projectile
                    break;
                }
            }
        }
    }
    prev_lmb = mouse.btn_left;
}
