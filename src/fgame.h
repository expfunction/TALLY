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

typedef enum
{
    LEVEL_HUB = 0,
    LEVEL_MISSION1 = 1,
    LEVEL_MISSION2 = 2,
    LEVEL_MISSION3 = 3
} LevelID;

extern GameState g_state;
extern LevelID g_current_level;
extern int g_loyalty;
extern int g_doubt;
extern int g_ammo;
extern int g_health;

void fgame_init(void);
void fgame_load_level(LevelID level);
void fgame_update(void);

#endif
