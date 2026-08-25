#include "fplayr.h"

FPlayer g_player;

void player_init(void)
{
    g_player.w_pos.x = 0;
    g_player.w_pos.y = 0;
    g_player.w_pos.z = 0;
    g_player.w_rot.x = 0;
    g_player.w_rot.y = 0;
}

void player_update(Camera *cam)
{
    // Update camera based on player position/rotation
    // Update player based on input

    // Example hook to camera
    // cam->pos = g_player.w_pos;
}
