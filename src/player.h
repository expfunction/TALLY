#ifndef PLAYER_H
#define PLAYER_H

#include "../CORE/CYBER.H"
#include "../CORE/TYPES/GFX.H"

/**
 * @brief Player struct
 * @param w_pos: World position Q16.16
 * @param w_rot: World rotation Q16.16 euler
 */
typedef struct Player_s
{
    Vec4 w_pos;
    Vec2 w_rot;
} Player;

extern Player g_player;

void player_init(void);
void player_update(Camera* cam);

#endif
