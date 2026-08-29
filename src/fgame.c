/* Copyright (c) 2026 Burak Yazar */

#include "fgame.h"
#include "flevel.h"
#include "IO/IO.H"
#include <stdio.h>

GameState g_state = STATE_TITLE;
LevelID g_current_level = LEVEL_BARRACKS;

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
    g_state = STATE_PLAYING;
    g_current_level = LEVEL_BARRACKS;
    g_loyalty = 0;
    g_doubt = 0;
    g_boxer_dead = 0;

    g_record_kept = 0;
    g_access_card_given = 0;
    g_assume_control = 0;

    g_ammo = 10; // Starting ammo
    g_health = 100;
    g_ending = 0;
    g_mission_progress = 1;

    // Init entity resources
    fentity_init();

    // Ensure the level loads when skipping the title screen
    flevel_init();

    fgame_load_level(LEVEL_BARRACKS);
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
}

void fgame_update(void)
{
    // Handle game state transitions and logic
    if (g_state == STATE_TITLE)
    {
        // Pressing Enter starts the game
        if (cv_io_key_pressed_now(KEY_ENTER))
        {
            g_state = STATE_PLAYING;
            fgame_load_level(LEVEL_BARRACKS);
        }
    }
}
