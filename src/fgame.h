#ifndef FGAME_H
#define FGAME_H

typedef enum
{
    STATE_TITLE,
    STATE_MENU,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_WIN,
    STATE_HELP
} GameState;

extern GameState g_state;
extern int g_loyalty;
extern int g_doubt;
extern int g_ammo;
extern int g_health;

void game_init(void);
void game_update(void);

#endif
