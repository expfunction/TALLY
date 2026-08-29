/* Copyright (c) 2026 Burak Yazar */

#include "fui.h"
#include "fgame.h"
#include "fentity.h"
#include "fplayr.h"
#include "CORE/CYBER.H"
#include "RNDR/FONT.H"
#include "RNDR/GUI.H"
#include <stdio.h>
#include <string.h>

// UI State
static int unit4_star_timer = 0;
static int display_ammo = 0;

// Need to track when he betrayed
static int unit4_betrayal_frame = -1;

void ui_init(void)
{
    unit4_star_timer = 0;
    unit4_betrayal_frame = -1;
    display_ammo = g_ammo;
}

void ui_update(void)
{
    int target_display_ammo = g_ammo;
    if (g_loyalty >= 2)
    {
        target_display_ammo += 5; // The lie
    }

    display_ammo = target_display_ammo;

    FEntity *ents = fentity_get_all();
    for (int i = 0; i < 128; i++)
    {
        if (!ents[i].active)
            continue;

        if (ents[i].type == ENT_TYPE_UNIT4)
        {
            if (ents[i].state == STATE_BETRAYAL)
            {
                if (unit4_betrayal_frame == -1)
                {
                    unit4_betrayal_frame = g_clock.frame; // mark the betrayal moment
                }
            }
            else
            {
                unit4_betrayal_frame = -1;
            }
        }
    }
}

void ui_draw(void)
{
    if (g_state != STATE_PLAYING)
        return;

    u8 *back = vga_backbuffer();

    // Draw Crosshair
    gui_fill_rect(back, 160 - 2, 120, 160 + 2, 120, 31);
    gui_fill_rect(back, 160, 120 - 2, 160, 120 + 2, 31);

    // Draw Health
    char health_str[32];
    sprintf(health_str, "HEALTH: %d", g_health);
    draw_text(back, 10, 220, health_str, BASIC_8, 31, -1);

    // Draw Ammo
    char ammo_str[32];
    sprintf(ammo_str, "AMMO: %d", display_ammo);
    draw_text(back, 250, 220, ammo_str, BASIC_8, 31, -1);

    // Radar Background
    int r_cx = 40, r_cy = 40, r_rad = 30;
    gui_panel(back, r_cx - r_rad, r_cy - r_rad, r_cx + r_rad, r_cy + r_rad, 0, 31, 31);
    draw_text(back, 12, 12, "RADAR", BASIC_4, 31, -1);

    // Draw entities on Radar and Unit 4's star
    FEntity *ents = fentity_get_all();
    for (int i = 0; i < 128; i++)
    {
        if (!ents[i].active)
            continue;

        // Relative position
        Vec4 dir;
        vec4_sub(&ents[i].pos, &g_player.w_pos, &dir);

        // 2D distance for radar scaling (1 unit = 2 pixels)
        int dx = FX_TO_INT(dir.x) * 2;
        int dz = FX_TO_INT(dir.z) * 2;

        // Rotate by player yaw to align radar forward
        i32 s = fx_sin(g_player.w_rot.y);
        i32 c = fx_cos(g_player.w_rot.y);

        int r_x = FX_TO_INT(fx_mul_q16(FX_FROM_INT(dx), c) - fx_mul_q16(FX_FROM_INT(dz), s));
        int r_y = FX_TO_INT(fx_mul_q16(FX_FROM_INT(dx), s) + fx_mul_q16(FX_FROM_INT(dz), c));

        // Clamp to radar bounds
        if (r_x < -r_rad)
            r_x = -r_rad;
        if (r_x > r_rad)
            r_x = r_rad;
        if (r_y < -r_rad)
            r_y = -r_rad;
        if (r_y > r_rad)
            r_y = r_rad;

        int px = r_cx + r_x;
        int py = r_cy + r_y; // Since Z is forward, mapped to Y

        // Fake map pings: Medkits show up as "TRAITOR" (red dot) when Loyalty is high
        if (ents[i].type == ENT_TYPE_MEDKIT)
        {
            if (g_loyalty >= 2)
            {
                gui_fill_rect(back, px - 1, py - 1, px + 1, py + 1, 40); // 40 = red
                draw_text(back, px + 3, py - 2, "TRAITOR", BASIC_4, 40, -1);
            }
            else
            {
                draw_text(back, px - 2, py - 2, "+", BASIC_8, 31, -1); // 31 = white
            }
        }

        // Enforcers are blue
        if (ents[i].type == ENT_TYPE_ENFORCER_F)
        {
            gui_fill_rect(back, px - 1, py - 1, px + 1, py + 1, 32); // 32 = blueish
        }

        // Unit 4
        if (ents[i].type == ENT_TYPE_UNIT4)
        {
            int draw_star = 1;

            if (ents[i].state == STATE_BETRAYAL)
            {
                // Keep the star for 3 seconds (90 frames at 30fps, or using g_clock.time)
                // Let's assume 30fps -> 90 frames
                if (unit4_betrayal_frame != -1 && (g_clock.frame - unit4_betrayal_frame) > 90)
                {
                    draw_star = 0; // The lie drops!
                }
            }

            if (draw_star)
            {
                draw_text(back, px - 2, py - 2, "*", BASIC_8, 43, -1); // 43 = gold/yellow
                draw_text(back, px + 4, py - 2, "ALLY", BASIC_4, 43, -1);
            }
            else
            {
                gui_fill_rect(back, px - 1, py - 1, px + 1, py + 1, 40); // Turn to generic red enemy
            }
        }
    }

    static u8 txx[32];
    sprintf(txx, "Plyr:%3.2f %3.2f %3.2f", FX_TO_FLOAT(g_player.w_pos.x),
            FX_TO_FLOAT(g_player.w_pos.y), FX_TO_FLOAT(g_player.w_pos.z));
    draw_text(back, 100, 210, txx, BASIC_8, 255, -1);
}
