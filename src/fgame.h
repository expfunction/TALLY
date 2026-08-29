/* Copyright (c) 2026 Burak Yazar */

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
    LEVEL_BARRACKS = 0,
    LEVEL_RATIONBLOCK = 1,
    LEVEL_GENERATOR = 2
} LevelID;

extern GameState g_state;
extern LevelID g_current_level;

// 1st Gen Compatibility
extern int g_loyalty;
extern int g_doubt;
extern int g_boxer_dead;

// 3rd Gen States
extern int g_record_kept;
extern int g_access_card_given;
extern int g_assume_control;

extern int g_ammo;
extern int g_health;
extern int g_ending;
extern int g_mission_progress;

void fgame_init(void);
void fgame_load_level(LevelID level);
void fgame_update(void);

#endif
