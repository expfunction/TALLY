#include "fgame.h"
#include "flevel.h"
#include "fplayr.h"
#include "fui.h"
#include "faudio.h"
#include "fentity.h"
#include "fsave.h"
#include "IO/IO.H"
#include <stdio.h>

extern int crt_on;
extern int aberr_on;
extern int smear_on;

#ifdef PLATFORM_WIN64
void vga_set_shader_warp(float warp_x, float warp_y);
void vga_set_shader_mask(float mask_dark, float mask_light);
void vga_set_shader_aberration(float shift_x, float shift_y);
void vga_set_shader_scanline(float weight);
void vga_set_shader_smear(float smear);
#endif

GameState g_state = STATE_SPLASH;
GameState g_previous_state = STATE_TITLE;
LevelID g_current_level = LEVEL_BARRACKS;
static i32 s_splash_timer = 0;

int g_settings_sel = 0;

// 1st Gen Compatibility
int g_loyalty = 0;
int g_doubt = 0;
int g_boxer_dead = 0;

// 3rd Gen States
int g_record_kept = 0;
int g_access_card_given = 0;
int g_assume_control = 0;

int g_ammo = 10;
int g_health = 100;
int g_ending = 0;
int g_mission_progress = 1;

void fgame_init(void)
{
    console_log("Game init");
    g_state = STATE_SPLASH;
    g_previous_state = STATE_TITLE;
    s_splash_timer = 0;
    cv_io_mouse_set_mode(CV_MOUSE_MODE_CURSOR);

    // Init UI & Audio
    ui_init();
    audio_init();

    // Init player & entity resources
    fplayer_init();
    fentity_init();

    // Init level structures
    flevel_init();
}

void fgame_start_game(void)
{
    audio_stop_title_music();
    g_state = STATE_PLAYING;
    cv_io_mouse_set_mode(CV_MOUSE_MODE_MOUSELOOK);

    g_current_level = LEVEL_BARRACKS;
    g_loyalty = 0;
    g_doubt = 0;
    g_boxer_dead = 0;

    g_record_kept = 0;
    g_access_card_given = 0;
    g_assume_control = 0;

    g_ammo = 10;
    g_health = 100;
    g_ending = 0;
    g_mission_progress = 1;

    fplayer_init();
    ui_init();
    level_swap_wall_ending(1);
    fgame_load_level(LEVEL_BARRACKS);

    // Initial Directorate broadcast orders
    ui_set_subtitle("DIRECTORATE BROADCAST", "Unit 9: Deploy to Sector 9.", "Investigate ration ledger discrepancy. Silence unrest.", 540, 43);
}

void fgame_continue_game(void)
{
    if (!fsave_exists())
    {
        fgame_start_game();
        return;
    }

    audio_stop_title_music();
    g_state = STATE_PLAYING;
    cv_io_mouse_set_mode(CV_MOUSE_MODE_MOUSELOOK);

    fsave_load();

    fplayer_init();
    ui_init();
    level_swap_wall_ending(1);
    fgame_load_level(g_current_level);

    if (g_current_level == LEVEL_BARRACKS)
    {
        ui_set_subtitle("DIRECTORATE BROADCAST", "Deployment Resumed: Sector 9 Barracks.", "Maintain sector security standards.", 480, 43);
    }
    else if (g_current_level == LEVEL_RATIONBLOCK)
    {
        ui_set_subtitle("DIRECTORATE BROADCAST", "Deployment Resumed: Ration Distribution Block.", "Investigate ledger discrepancy.", 480, 43);
    }
    else if (g_current_level == LEVEL_GENERATOR)
    {
        ui_set_subtitle("DIRECTORATE BROADCAST", "Deployment Resumed: Generator Sub-Core.", "Resolve laborer sabotage immediately.", 480, 43);
    }
}

void fgame_load_level(LevelID level)
{
    g_current_level = level;
    switch (level)
    {
    case LEVEL_BARRACKS:
        flevel_load("ASSTS\\WORLD\\BARRACKS.CWR");
        break;
    case LEVEL_RATIONBLOCK:
        flevel_load("ASSTS\\WORLD\\RATION.CWR");
        break;
    case LEVEL_GENERATOR:
        flevel_load("ASSTS\\WORLD\\GENERATOR.CWR");
        break;
    }
    fsave_checkpoint();
}

void fgame_update(void)
{
    // Handle game state transitions and logic using single-stroke detection
    if (g_state == STATE_SPLASH)
    {
        s_splash_timer += g_clock.dt;
        if (s_splash_timer >= FX_FROM_FLOAT(2.0f) ||
            cv_io_key_pressed_now(KEY_ENTER) || cv_io_key_pressed_now(KEY_SPACE) || cv_io_key_pressed_now(KEY_ESC))
        {
            g_state = STATE_TITLE;
        }
    }
    else if (g_state == STATE_TITLE)
    {
        int has_save = fsave_exists();
        if (cv_io_key_pressed_now(KEY_ENTER))
        {
            audio_play_sfx(SFX_DOOR_OPEN);
            if (has_save)
            {
                fgame_continue_game();
            }
            else
            {
                fgame_start_game();
            }
        }
        else if (has_save && cv_io_key_pressed_now(KEY_N))
        {
            fsave_delete();
            audio_play_sfx(SFX_DOOR_OPEN);
            fgame_start_game();
        }
        else if (cv_io_key_pressed_now(KEY_O))
        {
            audio_play_sfx(SFX_TERMINAL);
            g_previous_state = STATE_TITLE;
            g_state = STATE_SETTINGS;
        }
        else if (cv_io_key_pressed_now(KEY_H))
        {
            audio_play_sfx(SFX_TERMINAL);
            g_previous_state = STATE_TITLE;
            g_state = STATE_HELP;
        }
        else if (cv_io_key_pressed_now(KEY_ESC))
        {
            audio_play_sfx(SFX_TERMINAL);
            g_state = STATE_QUIT;
        }
    }
    else if (g_state == STATE_HELP)
    {
        if (cv_io_key_pressed_now(KEY_ENTER) || cv_io_key_pressed_now(KEY_H) ||
            cv_io_key_pressed_now(KEY_ESC) || cv_io_key_pressed_now(KEY_SPACE))
        {
            audio_play_sfx(SFX_TERMINAL);
            g_state = g_previous_state;
            if (g_state == STATE_PLAYING)
            {
                cv_io_mouse_set_mode(CV_MOUSE_MODE_MOUSELOOK);
            }
        }
    }
    else if (g_state == STATE_SETTINGS)
    {
        // Navigate settings menu
        if (cv_io_key_pressed_now(KEY_UP) || cv_io_key_pressed_now(KEY_W))
        {
            audio_play_sfx(SFX_TERMINAL);
            g_settings_sel--;
            if (g_settings_sel < 0)
                g_settings_sel = 4;
        }
        else if (cv_io_key_pressed_now(KEY_DOWN) || cv_io_key_pressed_now(KEY_S))
        {
            audio_play_sfx(SFX_TERMINAL);
            g_settings_sel++;
            if (g_settings_sel > 4)
                g_settings_sel = 0;
        }
        else if (cv_io_key_pressed_now(KEY_LEFT) || cv_io_key_pressed_now(KEY_A))
        {
            audio_play_sfx(SFX_TERMINAL);
            if (g_settings_sel == 0)
            {
                // Mouse Sensitivity
                g_mouse_sensitivity -= 16384; // 0.25 steps
                if (g_mouse_sensitivity < 16384)
                    g_mouse_sensitivity = 16384; // min 0.25x
            }
            else if (g_settings_sel == 1)
            {
                crt_on = !crt_on;
                if (crt_on)
                {
                    vga_set_shader_warp(1.0f / 64.0f, 1.0f / 48.0f);
                    vga_set_shader_mask(0.7f, 1.3f);
                    vga_set_shader_scanline(-12.0f);
                }
                else
                {
                    vga_set_shader_warp(0.0f, 0.0f);
                    vga_set_shader_mask(1.0f, 1.0f);
                    vga_set_shader_scanline(0.0f);
                }
            }
            else if (g_settings_sel == 2)
            {
                aberr_on = !aberr_on;
                if (aberr_on)
                    vga_set_shader_aberration(0.5f, 0.1f);
                else
                    vga_set_shader_aberration(0.0f, 0.0f);
            }
            else if (g_settings_sel == 3)
            {
                smear_on = !smear_on;
                if (smear_on)
                    vga_set_shader_smear(1.0f);
                else
                    vga_set_shader_smear(0.0f);
            }
            else if (g_settings_sel == 4)
            {
                int new_vol = g_master_volume - 25;
                audio_set_master_volume(new_vol);
            }
        }
        else if (cv_io_key_pressed_now(KEY_RIGHT) || cv_io_key_pressed_now(KEY_D) || cv_io_key_pressed_now(KEY_ENTER))
        {
            audio_play_sfx(SFX_TERMINAL);
            if (g_settings_sel == 0)
            {
                // Mouse Sensitivity
                g_mouse_sensitivity += 16384;
                if (g_mouse_sensitivity > 163840) // max 2.5x
                    g_mouse_sensitivity = 163840;
            }
            else if (g_settings_sel == 1)
            {
                crt_on = !crt_on;
                if (crt_on)
                {
                    vga_set_shader_warp(1.0f / 64.0f, 1.0f / 48.0f);
                    vga_set_shader_mask(0.7f, 1.3f);
                    vga_set_shader_scanline(-12.0f);
                }
                else
                {
                    vga_set_shader_warp(0.0f, 0.0f);
                    vga_set_shader_mask(1.0f, 1.0f);
                    vga_set_shader_scanline(0.0f);
                }
            }
            else if (g_settings_sel == 2)
            {
                aberr_on = !aberr_on;
                if (aberr_on)
                    vga_set_shader_aberration(0.5f, 0.1f);
                else
                    vga_set_shader_aberration(0.0f, 0.0f);
            }
            else if (g_settings_sel == 3)
            {
                smear_on = !smear_on;
                if (smear_on)
                    vga_set_shader_smear(1.0f);
                else
                    vga_set_shader_smear(0.0f);
            }
            else if (g_settings_sel == 4)
            {
                int new_vol = g_master_volume + 25;
                audio_set_master_volume(new_vol);
            }
        }
        else if (cv_io_key_pressed_now(KEY_ESC) || cv_io_key_pressed_now(KEY_O) || cv_io_key_pressed_now(KEY_BACKSPACE))
        {
            audio_play_sfx(SFX_TERMINAL);
            g_state = g_previous_state;
            if (g_state == STATE_PLAYING)
            {
                cv_io_mouse_set_mode(CV_MOUSE_MODE_MOUSELOOK);
            }
        }
    }
    else if (g_state == STATE_PLAYING)
    {
        if (cv_io_key_pressed_now(KEY_ESC) || cv_io_key_pressed_now(KEY_P))
        {
            g_state = STATE_MENU;
            cv_io_mouse_set_mode(CV_MOUSE_MODE_CURSOR);
            audio_play_sfx(SFX_TERMINAL);
        }
        else if (cv_io_key_pressed_now(KEY_O))
        {
            audio_play_sfx(SFX_TERMINAL);
            g_previous_state = STATE_PLAYING;
            g_state = STATE_SETTINGS;
            cv_io_mouse_set_mode(CV_MOUSE_MODE_CURSOR);
        }
        else if (cv_io_key_pressed_now(KEY_H))
        {
            audio_play_sfx(SFX_TERMINAL);
            g_previous_state = STATE_PLAYING;
            g_state = STATE_HELP;
            cv_io_mouse_set_mode(CV_MOUSE_MODE_CURSOR);
        }
    }
    else if (g_state == STATE_MENU)
    {
        if (cv_io_key_pressed_now(KEY_ESC) || cv_io_key_pressed_now(KEY_ENTER) || cv_io_key_pressed_now(KEY_P))
        {
            g_state = STATE_PLAYING;
            cv_io_mouse_set_mode(CV_MOUSE_MODE_MOUSELOOK);
            audio_play_sfx(SFX_TERMINAL);
        }
        else if (cv_io_key_pressed_now(KEY_O))
        {
            audio_play_sfx(SFX_TERMINAL);
            g_previous_state = STATE_MENU;
            g_state = STATE_SETTINGS;
        }
        else if (cv_io_key_pressed_now(KEY_R))
        {
            audio_play_sfx(SFX_DOOR_OPEN);
            fgame_load_level(g_current_level);
            g_state = STATE_PLAYING;
            cv_io_mouse_set_mode(CV_MOUSE_MODE_MOUSELOOK);
        }
        else if (cv_io_key_pressed_now(KEY_Q))
        {
            g_state = STATE_TITLE;
            cv_io_mouse_set_mode(CV_MOUSE_MODE_CURSOR);
            audio_play_sfx(SFX_TERMINAL);
        }
    }
    else if (g_state == STATE_GAMEOVER)
    {
        if (cv_io_key_pressed_now(KEY_ENTER))
        {
            audio_play_sfx(SFX_DOOR_OPEN);
            if (fsave_exists())
            {
                fgame_continue_game();
            }
            else
            {
                fgame_start_game();
            }
        }
        else if (cv_io_key_pressed_now(KEY_ESC))
        {
            g_state = STATE_TITLE;
            cv_io_mouse_set_mode(CV_MOUSE_MODE_CURSOR);
            audio_play_sfx(SFX_TERMINAL);
        }
    }
    else if (g_state == STATE_WIN)
    {
        static int s_win_initialized = 0;
        if (!s_win_initialized)
        {
            s_win_initialized = 1;
            fsave_delete();
            level_swap_wall_ending(g_ending);
        }

        if (cv_io_key_down(KEY_SPACE))
        {
            // Fast forward credits scroll
            ui_accelerate_credits(1.5f);
        }

        if (cv_io_key_pressed_now(KEY_ENTER) || cv_io_key_pressed_now(KEY_ESC))
        {
            s_win_initialized = 0;
            g_state = STATE_TITLE;
            cv_io_mouse_set_mode(CV_MOUSE_MODE_CURSOR);
            audio_play_sfx(SFX_TERMINAL);
        }
    }
}

