#include "fui.h"
#include "fgame.h"
#include "fentity.h"
#include "fplayr.h"
#include "flevel.h"
#include "faudio.h"
#include "CORE/CYBER.H"
#include "CORE/MTEXTUR.H"
#include "RNDR/FONT.H"
#include "RNDR/GUI.H"
#include "RNDR/SPRIT.H"
#include <stdio.h>
#include <string.h>

// UI State
static int unit4_star_timer = 0;
static int display_ammo = 0;
int unit4_betrayal_frame = -1;

static int s_damage_flash_timer = 0;
static int s_muzzle_flash_timer = 0;

extern int crt_on;
extern int aberr_on;
extern int smear_on;
extern int g_settings_sel;

#include "fsave.h"

// Radio Broadcast Icon
static Sprite s_radio_icon = {0};
static int s_radio_icon_loaded = 0;

// Subtitle / Narrative State
static char s_speaker[64] = {0};
static char s_line1[128] = {0};
static char s_line2[128] = {0};
static int s_subtitle_timer = 0;
static u8 s_speaker_color = 43;

// Contextual Interaction Prompt
static char s_prompt[64] = {0};
static int s_prompt_active = 0;

// Ending Credits Scroll Position
static float s_credits_scroll = 0;

void ui_trigger_damage_flash(void)
{
    s_damage_flash_timer = 8;
}

void ui_trigger_muzzle_flash(void)
{
    s_muzzle_flash_timer = 4;
}

void ui_accelerate_credits(float delta)
{
    s_credits_scroll += delta;
}

void ui_set_subtitle(const char *speaker, const char *line1, const char *line2, int duration_frames, u8 color)
{
    if (speaker)
        strncpy(s_speaker, speaker, sizeof(s_speaker) - 1);
    else
        s_speaker[0] = '\0';
    if (line1)
        strncpy(s_line1, line1, sizeof(s_line1) - 1);
    else
        s_line1[0] = '\0';
    if (line2)
        strncpy(s_line2, line2, sizeof(s_line2) - 1);
    else
        s_line2[0] = '\0';
    s_subtitle_timer = duration_frames;
    s_speaker_color = color;
}

void ui_set_prompt(const char *prompt_text)
{
    if (prompt_text)
    {
        strncpy(s_prompt, prompt_text, sizeof(s_prompt) - 1);
        s_prompt_active = 1;
    }
}

static u8 *s_cvga_bg = NULL;
static u8 *s_title_bg = NULL;
static u8 *s_gameover_bg = NULL;
static u8 *s_ending_a_bg = NULL;
static u8 *s_ending_b_bg = NULL;
static u8 *s_ending_c_bg = NULL;

static void ui_load_bg_once(const char *path1, const char *path2, u8 **cache)
{
    if (*cache)
        return;
    *cache = (u8 *)malloc(320 * 240);
    if (!*cache)
        return;
    if (!load_raw8(path1, *cache, 320, 240))
    {
        if (path2 && !load_raw8(path2, *cache, 320, 240))
        {
            memset(*cache, 0, 320 * 240);
        }
    }
}

void ui_init(void)
{
    unit4_star_timer = 0;
    unit4_betrayal_frame = -1;
    display_ammo = g_ammo;
    s_subtitle_timer = 0;
    s_prompt_active = 0;
    s_credits_scroll = 0;
    s_damage_flash_timer = 0;
    s_muzzle_flash_timer = 0;
    s_speaker[0] = '\0';
    s_line1[0] = '\0';
    s_line2[0] = '\0';

    if (!s_radio_icon_loaded)
    {
        if (load_sprite(&s_radio_icon, (const u8 *)"ASSTS\\HUD\\RADIOICO.RAW", 64, 128))
            s_radio_icon_loaded = 1;
        else if (load_sprite(&s_radio_icon, (const u8 *)"ASSTS\\HUD\\RADIO_ICO.RAW", 64, 128))
            s_radio_icon_loaded = 1;
        else
            s_radio_icon_loaded = -1; // Mark attempted so missing file never re-triggers disk I/O
    }

    ui_load_bg_once("ASSTS\\HUD\\CVGA.RAW", "ASSTS\\TEXTR\\CVGA.RAW", &s_cvga_bg);
    ui_load_bg_once("ASSTS\\HUD\\TITLE.RAW", "ASSTS\\TEXTR\\TITLE.RAW", &s_title_bg);
    ui_load_bg_once("ASSTS\\HUD\\GAMEOVER.RAW", "ASSTS\\TEXTR\\GAMEOVER.RAW", &s_gameover_bg);
    ui_load_bg_once("ASSTS\\HUD\\ENDING_A.RAW", "ASSTS\\TEXTR\\ENDING_A.RAW", &s_ending_a_bg);
    ui_load_bg_once("ASSTS\\HUD\\ENDING_B.RAW", "ASSTS\\TEXTR\\ENDING_B.RAW", &s_ending_b_bg);
    ui_load_bg_once("ASSTS\\HUD\\ENDING_C.RAW", "ASSTS\\TEXTR\\ENDING_C.RAW", &s_ending_c_bg);
}

void ui_update(void)
{
    if (s_damage_flash_timer > 0)
        s_damage_flash_timer--;
    if (s_muzzle_flash_timer > 0)
        s_muzzle_flash_timer--;

    if (s_subtitle_timer > 0)
    {
        // Hold open if radio broadcast is still speaking
        if (audio_is_radio_playing() && s_subtitle_timer < 180)
        {
            s_subtitle_timer = 180;
        }
        s_subtitle_timer--;
    }

    if (g_state == STATE_WIN)
    {
        s_credits_scroll += FX_TO_FLOAT(g_clock.dt) * 25.0f;
        return;
    }

    if (g_state != STATE_PLAYING && g_state != STATE_MENU)
        return;

    // Lighting as Morality: WARM light equals Party comfort/lies. COLD light equals truth.
    int is_warm = flevel_is_in_warm_light(g_player.w_pos);

    int target_display_ammo = g_ammo;
    if (is_warm && g_loyalty >= 2)
    {
        target_display_ammo += 5; // The lie inside institutional warm light
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
                    unit4_betrayal_frame = g_clock.frame;
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
    u8 *back = vga_backbuffer();

    // -------------------------------------------------------------
    // CVGA SPLASH SCREEN
    // -------------------------------------------------------------
    if (g_state == STATE_SPLASH)
    {
        if (s_cvga_bg)
        {
            memcpy(back, s_cvga_bg, 320 * 240);
        }
        return;
    }

    // -------------------------------------------------------------
    // TITLE SCREEN
    // -------------------------------------------------------------
    if (g_state == STATE_TITLE)
    {
        if (s_title_bg)
        {
            memcpy(back, s_title_bg, 320 * 240);
        }

        int has_save = fsave_exists();
        int blink = (g_clock.frame / 20) % 2;

        if (has_save)
        {
            if (blink)
            {
                draw_text(back, 50, 192, "> [ENTER] RESUME DUTY (CONTINUE) <", BASIC_5, 43, -1);
            }
            else
            {
                draw_text(back, 50, 192, "  [ENTER] RESUME DUTY (CONTINUE)  ", BASIC_5, 31, -1);
            }
            draw_text(back, 50, 203, "[N] Start Fresh Operation", BASIC_5, 31, -1);
            draw_text(back, 50, 214, "[O] Sector Configuration (Options)", BASIC_5, 31, -1);
            draw_text(back, 50, 225, "[H] Assistance Manual    [ESC] Quit", BASIC_5, 31, -1);
        }
        else
        {
            if (blink)
            {
                draw_text(back, 56, 198, "> [ENTER] CLOCK IN (NEW MISSION) <", BASIC_5, 43, -1);
            }
            else
            {
                draw_text(back, 56, 198, "  [ENTER] CLOCK IN (NEW MISSION)  ", BASIC_5, 31, -1);
            }
            draw_text(back, 56, 212, "[O] Sector Configuration (Options)", BASIC_5, 31, -1);
            draw_text(back, 56, 224, "[H] Assistance Manual    [ESC] Quit", BASIC_5, 31, -1);
        }
        return;
    }

    // -------------------------------------------------------------
    // SECTOR CONFIGURATION (SETTINGS / OPTIONS)
    // -------------------------------------------------------------
    if (g_state == STATE_SETTINGS)
    {
        gui_panel(back, 18, 14, 302, 226, 0, 31, 43);
        draw_text(back, 36, 24, "SECTOR CONFIGURATION [OPTIONS]", BASIC_8, 43, -1);

        // 0: Mouse Sensitivity
        char sens_str[48];
        float sens_f = FX_TO_FLOAT(g_mouse_sensitivity);
        sprintf(sens_str, "Look Sensitivity    : < %.2fx >", sens_f);
        draw_text(back, 28, 55, sens_str, BASIC_5, (g_settings_sel == 0) ? 43 : 31, -1);

        // 1: CRT Monitor Filter
        char crt_str[48];
        sprintf(crt_str, "CRT Monitor Scanline: < %s >", crt_on ? "ENABLED" : "DISABLED");
        draw_text(back, 28, 75, crt_str, BASIC_5, (g_settings_sel == 1) ? 43 : 31, -1);

        // 2: Chromatic Aberration
        char aber_str[48];
        sprintf(aber_str, "Chromatic Optics    : < %s >", aberr_on ? "ENABLED" : "DISABLED");
        draw_text(back, 28, 95, aber_str, BASIC_5, (g_settings_sel == 2) ? 43 : 31, -1);

        // 3: Phosphor Smear
        char smear_str[48];
        sprintf(smear_str, "Phosphor Screen Smear: < %s >", smear_on ? "ENABLED" : "DISABLED");
        draw_text(back, 28, 115, smear_str, BASIC_5, (g_settings_sel == 3) ? 43 : 31, -1);

        // 4: Master Volume
        char vol_str[48];
        sprintf(vol_str, "Master Audio Gain   : < %d%% >", g_master_volume);
        draw_text(back, 28, 135, vol_str, BASIC_5, (g_settings_sel == 4) ? 43 : 31, -1);

        draw_text(back, 28, 172, "[UP / DOWN] Select Directive", BASIC_5, 31, -1);
        draw_text(back, 28, 184, "[LEFT / RIGHT / ENTER] Modify Parameter", BASIC_5, 31, -1);
        draw_text(back, 44, 210, "[ESC / O / BACKSPACE TO RETURN]", BASIC_8, 43, -1);
        return;
    }

    // -------------------------------------------------------------
    // ASSISTANCE / HELP SCREEN
    // -------------------------------------------------------------
    if (g_state == STATE_HELP)
    {
        gui_panel(back, 18, 14, 302, 226, 0, 31, 43);
        draw_text(back, 42, 24, "SECTOR NINE ASSISTANCE MANUAL", BASIC_8, 43, -1);

        draw_text(back, 28, 48, "OPERATIONAL DIRECTIVES:", BASIC_8, 43, -1);
        draw_text(back, 28, 60, "1. Investigate ration ledger discrepancy in Sector 9.", BASIC_5, 31, -1);
        draw_text(back, 28, 72, "2. Obey Directorate orders broadcast over the radio.", BASIC_5, 31, -1);
        draw_text(back, 28, 84, "3. Do not trust the radio. Do not trust the wall.", BASIC_5, 40, -1);

        draw_text(back, 28, 106, "FIELD ENFORCER CONTROLS:", BASIC_8, 43, -1);
        draw_text(back, 28, 118, "WASD        : Move / Strafe", BASIC_5, 31, -1);
        draw_text(back, 28, 130, "Mouse       : Look & Aim", BASIC_5, 31, -1);
        draw_text(back, 28, 142, "Left Click  : Fire Suppressed Pistol", BASIC_5, 31, -1);
        draw_text(back, 28, 154, "E           : Interact (Doors, Ledger, Terminals)", BASIC_5, 31, -1);
        draw_text(back, 28, 166, "ESC / P     : Pause Sector Compliance", BASIC_5, 31, -1);

        draw_text(back, 48, 198, "[PRESS ENTER OR H TO RETURN]", BASIC_8, 43, -1);
        return;
    }

    // -------------------------------------------------------------
    // IN-GAME PAUSE MENU
    // -------------------------------------------------------------
    if (g_state == STATE_MENU)
    {
        gui_panel(back, 45, 30, 275, 210, 0, 31, 43);
        draw_text(back, 62, 45, "SECTOR OPERATION PAUSED", BASIC_8, 43, -1);

        draw_text(back, 60, 72, "STATUS REPORT:", BASIC_5, 43, -1);
        char lvl_str[48];
        sprintf(lvl_str, "Sector Area : %s",
                (g_current_level == LEVEL_BARRACKS) ? "Barracks Hub" : ((g_current_level == LEVEL_RATIONBLOCK) ? "Ration Block" : "Generator Core"));
        draw_text(back, 60, 86, lvl_str, BASIC_5, 31, -1);

        char stat_str[48];
        sprintf(stat_str, "Health: %d | Ammo: %d", g_health, display_ammo);
        draw_text(back, 60, 100, stat_str, BASIC_5, 31, -1);

        draw_text(back, 60, 130, "[ESC / ENTER] Resume Duty", BASIC_5, 43, -1);
        draw_text(back, 60, 146, "[O] Sector Options (Settings)", BASIC_5, 31, -1);
        draw_text(back, 60, 162, "[R] Restart Sector Mission", BASIC_5, 31, -1);
        draw_text(back, 60, 178, "[Q] Clock Out (Title Screen)", BASIC_5, 31, -1);
        return;
    }

    // -------------------------------------------------------------
    // GAME OVER SCREEN (320x240 RAW support)
    // -------------------------------------------------------------
    if (g_state == STATE_GAMEOVER)
    {
        if (s_gameover_bg)
        {
            memcpy(back, s_gameover_bg, 320 * 240);
        }

        gui_panel(back, 25, 30, 295, 210, 0, 31, 40);
        draw_text(back, 42, 45, "DIRECTORATE INCIDENT REPORT", BASIC_8, 40, -1);
        draw_text(back, 42, 68, "STATUS: UNIT 9 TERMINATED IN SERVICE", BASIC_8, 31, -1);

        draw_text(back, 42, 95, "Enforcer neutralized during sector operation.", BASIC_5, 31, -1);
        draw_text(back, 42, 110, "Physical discrepancy purged from records.", BASIC_5, 31, -1);
        draw_text(back, 42, 125, "The Directorate will dispatch Unit 10.", BASIC_5, 31, -1);
        draw_text(back, 42, 145, "\"EVERY CITIZEN IS COUNTED.\"", BASIC_5, 43, -1);

        draw_text(back, 48, 175, "[ENTER] CLOCK IN AGAIN (RESTART)", BASIC_5, 43, -1);
        draw_text(back, 48, 190, "[ESC] RETURN TO CLOCK-IN SCREEN", BASIC_5, 31, -1);
        return;
    }

    // -------------------------------------------------------------
    // 3 ENDING SCREENS (320x240 RAW + Sliding Text & Credits)
    // -------------------------------------------------------------
    if (g_state == STATE_WIN)
    {
        u8 *end_bg = (g_ending == 2) ? s_ending_b_bg : ((g_ending == 3) ? s_ending_c_bg : s_ending_a_bg);
        if (end_bg)
        {
            memcpy(back, end_bg, 320 * 240);
        }

        static const char *lines_ending1[] = {
            "DIRECTORATE PERSONNEL ASSESSMENT",
            "CLASSIFICATION: RELIABLE",
            "----------------------------------------",
            "The physical ledger was destroyed.",
            "The Forced Labourer was detained in the van.",
            "The Barracks commandment loses its text.",
            "",
            "DIRECTORATE BROADCAST:",
            "\"Thank you Unit 9 for correcting the record.\"",
            "FINAL WALL STATUS: [BLANK / TEXTLESS]",
            "",
            "========================================",
            "CREDITS",
            "========================================",
            "COUNTED [TRUST NO ONE]",
            "A Novus Idea Production",
            "",
            "Programming & Systems : Burak Yazar",
            "Narrative & Design & Music : Berk Yazar",
            "Engine Architecture : CyberVGA (Win64 / DOS)",
            "Models & Rigs : Max Parata (Mixamo)",
            "Audio & Sound Blaster : SMIX Subsystem",
            "",
            "Created for Brackey's Jam 2026.2",
            "========================================",
            "PRESS ENTER OR ESC TO CLOCK OUT",
            NULL};

        static const char *lines_ending2[] = {
            "DIRECTORATE PERSONNEL ASSESSMENT",
            "CLASSIFICATION: UNTRUSTWORTHY",
            "----------------------------------------",
            "The Forced Labourer transmitted the ledger.",
            "Unit 9 clearance revoked. Unit 4 attacked.",
            "Physical evidence can no longer be erased.",
            "",
            "STATUS: ESCAPED VIA SERVICE ROUTE",
            "CIVIC RECORD WALL READS:",
            "\"THE RECORD SURVIVES.\"",
            "",
            "========================================",
            "CREDITS",
            "========================================",
            "COUNTED [TRUST NO ONE]",
            "A Novus Idea Production",
            "",
            "Programming & Systems : Burak Yazar",
            "Narrative & Design & Music : Berk Yazar",
            "Engine Architecture : CyberVGA (Win64 / DOS)",
            "Models & Rigs : Max Parata (Mixamo)",
            "Audio & Sound Blaster : SMIX Subsystem",
            "",
            "Created for Brackey's Jam 2026.2",
            "========================================",
            "PRESS ENTER OR ESC TO CLOCK OUT",
            NULL};

        static const char *lines_ending3[] = {
            "SYSTEM CONTINUITY PROTOCOL",
            "CLASSIFICATION: SUCCESSOR",
            "----------------------------------------",
            "Unit 9 seized the Continuity Terminal.",
            "The old leadership broadcast feed was cut.",
            "The mural transforms into Unit 9's face.",
            "",
            "DIRECTORATE BROADCAST:",
            "\"Repeating Unit 9's command.\"",
            "CIVIC RECORD WALL READS:",
            "\"ONLY THE AUTHOR COUNTS.\"",
            "",
            "========================================",
            "CREDITS",
            "========================================",
            "COUNTED [TRUST NO ONE]",
            "A Novus Idea Production",
            "",
            "Programming & Systems : Burak Yazar",
            "Narrative & Design & Music : Berk Yazar",
            "Engine Architecture : CyberVGA (Win64 / DOS)",
            "Models & Rigs : Max Parata (Mixamo)",
            "Audio & Sound Blaster : SMIX Subsystem",
            "",
            "Created for Brackey's Jam 2026.2",
            "========================================",
            "PRESS ENTER OR ESC TO CLOCK OUT",
            NULL};

        const char **cur_lines = (g_ending == 2) ? lines_ending2 : ((g_ending == 3) ? lines_ending3 : lines_ending1);
        u8 header_color = (g_ending == 2) ? 40 : 43;

        gui_panel(back, 15, 10, 305, 230, 0, 31, header_color);

        int start_y = 210 - (int)(s_credits_scroll / 2.0f);

        int idx = 0;
        while (cur_lines[idx] != NULL)
        {
            int line_y = start_y + (idx * 11);
            if (line_y >= 20 && line_y <= 214)
            {
                if (idx == 0 || idx == 1 || idx == 12 || idx == 14)
                {
                    draw_text(back, 24, line_y, cur_lines[idx], BASIC_5, header_color, -1);
                }
                else
                {
                    draw_text(back, 24, line_y, cur_lines[idx], BASIC_5, 31, -1);
                }
            }
            idx++;
        }

        draw_text(back, 48, 218, "[PRESS ENTER OR ESC TO CLOCK OUT]", BASIC_5, header_color, -1);
        return;
    }

    if (g_state != STATE_PLAYING)
        return;

    // Red Damage Vignette Flash when struck
    if (s_damage_flash_timer > 0)
    {
        gui_fill_rect(back, 0, 0, 319, 2, 40);
        gui_fill_rect(back, 0, 237, 319, 239, 40);
        gui_fill_rect(back, 0, 0, 2, 239, 40);
        gui_fill_rect(back, 317, 0, 319, 239, 40);
    }

    // Draw Crosshair (with bright pulse if firing)
    if (s_muzzle_flash_timer > 0)
    {
        gui_fill_rect(back, 160 - 3, 119, 160 + 3, 121, 43);
        gui_fill_rect(back, 159, 120 - 3, 161, 120 + 3, 43);
        gui_fill_rect(back, 180, 215, 186, 221, 43);
        gui_fill_rect(back, 182, 213, 188, 219, 31);
    }
    else
    {
        gui_fill_rect(back, 160 - 2, 120, 160 + 2, 120, 31);
        gui_fill_rect(back, 160, 120 - 2, 160, 120 + 2, 31);
    }

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
    draw_text(back, 12, 12, "RADAR", BASIC_5, 31, -1);

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
        int py = r_cy - r_y;

        // Ammo pack on radar (gold dot)
        if (ents[i].type == ENT_TYPE_AMMO)
        {
            gui_fill_rect(back, px - 1, py - 1, px + 1, py + 1, 43);
            draw_text(back, px + 3, py - 2, "AMMO", BASIC_5, 43, -1);
        }

        // Fake map pings: Medkits show up as "TRAITOR" (red dot) when Loyalty is high AND inside Warm Light
        if (ents[i].type == ENT_TYPE_MEDKIT)
        {
            int is_warm = flevel_is_in_warm_light(g_player.w_pos);
            if (is_warm && g_loyalty >= 2)
            {
                gui_fill_rect(back, px - 1, py - 1, px + 1, py + 1, 40); // 40 = red
                draw_text(back, px + 3, py - 2, "TRAITOR", BASIC_5, 40, -1);
            }
            else
            {
                draw_text(back, px - 2, py - 2, "+", BASIC_8, 31, -1); // 31 = white
            }
        }

        // Enforcers are blue
        if (ents[i].type == ENT_TYPE_ENFORCER_F || ents[i].type == ENT_TYPE_ENFORCER_M)
        {
            gui_fill_rect(back, px - 1, py - 1, px + 1, py + 1, 32); // 32 = blueish
        }

        // Unit 4
        if (ents[i].type == ENT_TYPE_UNIT4)
        {
            int draw_star = 1;

            if (ents[i].state == STATE_BETRAYAL)
            {
                if (unit4_betrayal_frame != -1 && (g_clock.frame - unit4_betrayal_frame) > 90)
                {
                    draw_star = 0; // The lie drops after 3 seconds!
                }
            }

            if (draw_star)
            {
                draw_text(back, px - 2, py - 2, "*", BASIC_8, 43, -1); // 43 = gold/yellow
                draw_text(back, px + 4, py - 2, "ALLY", BASIC_5, 43, -1);
            }
            else
            {
                gui_fill_rect(back, px - 1, py - 1, px + 1, py + 1, 40); // Turn to generic red enemy
            }
        }
    }

    // Top Subtitle / Broadcast Notification Banner
    if (s_subtitle_timer > 0)
    {
        gui_panel(back, 15, 6, 305, 42, 0, 31, s_speaker_color);

        int text_x = 22;
        if (s_radio_icon_loaded > 0 && s_radio_icon.pix)
        {
            int frame_y = ((g_clock.frame / 20) % 2) * 64;
            ClipRect clip = {0, 0, FX_FROM_INT(320), FX_FROM_INT(240)};
            draw_sprite_scaled_sub(back, 30, 24, 26, 26,
                                   &s_radio_icon, 0, frame_y, 64, 64,
                                   255, 0, 0, &clip, 0);
            text_x = 48;
        }

        draw_text(back, text_x, 11, s_speaker, BASIC_5, s_speaker_color, -1);
        if (s_line1[0])
            draw_text(back, text_x, 21, s_line1, BASIC_5, 31, -1);
        if (s_line2[0])
            draw_text(back, text_x, 30, s_line2, BASIC_5, 31, -1);
    }
    else if (audio_is_radio_playing() && s_radio_icon_loaded > 0 && s_radio_icon.pix)
    {
        // Live broadcast transmission indicator
        int frame_y = ((g_clock.frame / 15) % 2) * 64;
        ClipRect clip = {0, 0, FX_FROM_INT(320), FX_FROM_INT(240)};
        gui_panel(back, 215, 6, 305, 32, 0, 31, 32);
        draw_sprite_scaled_sub(back, 227, 19, 20, 20,
                               &s_radio_icon, 0, frame_y, 64, 64,
                               255, 0, 0, &clip, 0);
        draw_text(back, 240, 11, "DIRECTORATE", BASIC_5, 43, -1);
        draw_text(back, 240, 20, "BROADCAST", BASIC_5, 31, -1);
    }

    // Contextual Interaction Prompt (Bottom Center)
    if (s_prompt_active && s_prompt[0])
    {
        int len = (int)strlen(s_prompt);
        int pw = len * 6 + 12;
        int px0 = 160 - (pw / 2);
        int px1 = 160 + (pw / 2);
        if (px0 < 10)
            px0 = 10;
        if (px1 > 310)
            px1 = 310;
        gui_panel(back, px0, 186, px1, 202, 0, 31, 43);
        draw_text(back, px0 + 6, 190, s_prompt, BASIC_5, 43, -1);
        s_prompt_active = 0; // Reset for next frame
    }
}
