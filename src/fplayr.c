#include "fplayr.h"
#include "IO/IO.H"
#include "RNDR/CAMER.H"
#include "PHYS/PHYS.H"
#include "CORE/CYBER.H"

FPlayer g_player;

void player_init(void)
{
    g_player.w_pos.x = 0;
    g_player.w_pos.y = FX_FROM_FLOAT(1.5f); // Start a bit above ground
    g_player.w_pos.z = 0;
    g_player.w_pos.w = FX_ONE;
    g_player.w_rot.x = 0; // pitch
    g_player.w_rot.y = 0; // yaw
}

void player_update(Camera *cam)
{
    // 1. Mouse look
    MouseIO mouse;
    if (cv_io_mouse_state(&mouse))
    {
        // dx and dy are integer screen deltas
        g_player.w_rot.y -= FX_FROM_INT(mouse.dx) / 100; // Yaw
        g_player.w_rot.x -= FX_FROM_INT(mouse.dy) / 100; // Pitch

        // Clamp pitch to avoid flipping
        i32 pitch_limit = FX_FROM_FLOAT(1.5f); // ~85 degrees
        if (g_player.w_rot.x > pitch_limit)
            g_player.w_rot.x = pitch_limit;
        if (g_player.w_rot.x < -pitch_limit)
            g_player.w_rot.x = -pitch_limit;
    }

    // 2. WASD Movement based on yaw
    i32 move_speed = fx_mul_q16(FX_FROM_FLOAT(4.0f), g_clock.dt); // 4 units per second
    i32 dx = 0;
    i32 dz = 0;

    if (cv_io_key_down(KEY_W))
        dz += move_speed;
    if (cv_io_key_down(KEY_S))
        dz -= move_speed;
    if (cv_io_key_down(KEY_A))
        dx -= move_speed;
    if (cv_io_key_down(KEY_D))
        dx += move_speed;

    // Apply movement relative to yaw
    if (dx != 0 || dz != 0)
    {
        i32 sin_y = fx_sin(g_player.w_rot.y);
        i32 cos_y = fx_cos(g_player.w_rot.y);

        // Forward/back
        g_player.w_pos.x += fx_mul_q16(dz, sin_y);
        g_player.w_pos.z += fx_mul_q16(dz, cos_y);

        // Strafe
        g_player.w_pos.x += fx_mul_q16(dx, cos_y);
        g_player.w_pos.z -= fx_mul_q16(dx, sin_y);
    }

    // Hook camera
    cam->position = g_player.w_pos;
    cam->pitch = g_player.w_rot.x;
    cam->yaw = g_player.w_rot.y;
    cam->roll = 0;

    cam_update(cam, CAM_PROJ_PERSPECTIVE);

    // Optional: Collide camera against the world walls using physics system
    phys_collide_camera(cam, &g_world);
    g_player.w_pos = cam->position; // Update back player position if pushed

    // Interaction check
    if (cv_io_key_pressed_now(KEY_E))
    {
        // TODO: Interaction screen to world ray casting
    }
}
