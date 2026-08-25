#ifndef FPLAYR_H
#define FPLAYR_H

#include "CORE/CYBER.H"
#include "CORE/TYPES/GFX.H"

/**
 * @brief Player struct
 * @param w_pos: World position Q16.16
 * @param w_rot: World rotation Q16.16 euler
 */
typedef struct FPlayer_s
{
    Vec4 w_pos;
    Vec2 w_rot;
} FPlayer;

extern FPlayer g_player;

void fplayer_init(void);
void fplayer_set_start_pos(Vec4 pos);
void fplayer_update(Camera *cam);

#endif
