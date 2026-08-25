#include "game.h"
#include <stdio.h>

GameState g_state = STATE_TITLE;
int g_loyalty = 0;
int g_doubt = 0;

void game_init(void)
{
    g_state = STATE_TITLE;
    g_loyalty = 0;
    g_doubt = 0;
}

void game_update(void)
{
    // Handle game state transitions and logic
}
