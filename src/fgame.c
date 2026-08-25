#include "fgame.h"
#include "flevel.h"
#include "IO/IO.H"
#include <stdio.h>

GameState g_state = STATE_TITLE;
LevelID g_current_level = LEVEL_HUB;
int g_loyalty = 0;
int g_doubt = 0;
int g_ammo = 10;
int g_health = 100;

void game_init(void)
{
    g_state = STATE_TITLE;
    g_current_level = LEVEL_HUB;
    g_loyalty = 0;
    g_doubt = 0;
    g_ammo = 10; // Starting ammo
    g_health = 100;
}

void game_load_level(LevelID level)
{
    g_current_level = level;
    switch (level)
    {
    case LEVEL_HUB:
        flevel_load("ASSTS\\MAPS\\MAP_BARN.CWR"); // Assuming this is the filename
        break;
    case LEVEL_MISSION1:
        flevel_load("ASSTS\\MAPS\\MAP_HENHOUSE.CWR");
        break;
    case LEVEL_MISSION2:
        flevel_load("ASSTS\\MAPS\\MAP_MINISTRY.CWR");
        break;
    case LEVEL_MISSION3:
        flevel_load("ASSTS\\MAPS\\MAP_MILL.CWR");
        break;
    }
}

void game_update(void)
{
    // Handle game state transitions and logic
    if (g_state == STATE_TITLE)
    {
        // Pressing Enter starts the game
        if (cv_io_key_pressed_now(KEY_ENTER))
        {
            g_state = STATE_PLAYING;
            game_load_level(LEVEL_HUB);
        }
    }
}
