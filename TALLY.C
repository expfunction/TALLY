/*
 * CyberVGA SDK - The Tally - Burak Yazar
 */

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include "CORE/CENG.H"
#include "CORE/CYBER.H"
#include "CORE/FIXED.H"
#include "CORE/PLATFRM.H"
#include "SOUND/BLAST.H"
#include "SOUND/SMIX.H"
#include "RNDR/GUI.H"
#include "RNDR/FONT.H"
#include "RNDR/SPRIT.H"
#include "CORE/TYPES/GFX.H"
#include "IO/IO.H"

#define SCREEN_W 320
#define SCREEN_H 240

/* Default sound config */
static BlastConfig g_blast_cfg = {
    0x220, 5, 1, 22050};

#include "src/fgame.h"
#include "src/fplayr.h"
#include "src/flevel.h"
#include "src/fentity.h"
#include "src/fui.h"
#include "src/faudio.h"

static PCM8UFile m_menu, m_01, m_02, m_03, m_04, m_05, m_06, m_over, m_win;
static int menuplayed = 0, m_01played = 0, m_02played = 0, m_03played = 0, m_04played = 0, m_05played = 0, m_06played = 0, gameoverplayed = 0, winplayed = 0;

/* Sprites */
Sprite sun_spr, moon_spr;

int main(int argc, char **argv)
{
    Camera cam;
    ClipRect screen_rect = {0, 0, FX_FROM_INT(SCREEN_W - 1), FX_FROM_INT(SCREEN_H)};
    Surface8 surf = {SCREEN_W, SCREEN_H, 0, 0};
    int loopdone = 0;

    (void)argc;
    (void)argv;

#ifdef PLATFORM_WIN64
    platfrm_chdir_project_root();
#endif

    init_console("THE TALLY - V0.1a Console Init");

    {
        EngCfg cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.title = "THE TALLY";
        cfg.pal_path = "ASSTS\\PAL\\TALLY.PAL";
        cfg.mouse_mode = CV_MOUSE_MODE_CURSOR;
        cfg.blast_cfg = &g_blast_cfg;
        cfg.flags = ENG_F_ENT | ENG_F_ANIM | ENG_F_SND_OPT;

        if (!engine_init(&cfg))
        {
            console_log("Engine init failed\n");
            return 1;
        }

        /* Load sky sprites*/
        if (!load_sprite(&sun_spr, "ASSTS\\TEXTR\\SKY\\SUN.RAW", 16, 16))
        {
            console_log("Sun raw not raw");
            goto QUIT;
        }
        if (!load_sprite(&moon_spr, "ASSTS\\TEXTR\\SKY\\MOON.RAW", 16, 16))
        {
            console_log("Moon raw not raw");
            goto QUIT;
        }
        sky_init(&sun_spr, &moon_spr);

        console_log("Audio systems init.");
        blast_init(&g_blast_cfg);
        smix_init(22050);
    }

    memset(&cam, 0, sizeof(cam));
    {
        Vec3 camp = {0, 0, 0};
        Vec3 camr = {0, 0, 0};
        cam_init(&cam, &camp, &camr, CAM_PROJ_PERSPECTIVE);
    }

    surf.back = vga_backbuffer();

    /* Title, background tile, game over, sounds */
    {
        fgame_init();
    }
    cv_io_keyboard_init();
    cv_io_mouse_init(SCREEN_W, SCREEN_H, CV_MOUSE_MODE_MOUSELOOK);

    flevel_init();

    while (!loopdone)
    {

        engine_update(&cam);

        cv_io_keyboard_poll();
        cv_io_mouse_poll();
        fgame_update();
        if (g_state == STATE_PLAYING)
        {
            player_update(&cam);
            flevel_update();
            fentity_update();
        }
        ui_update();
        audio_update();

        smix_update(&cam);
        if (cv_io_key_down(KEY_ESC))
        {

            loopdone = 1;
        }

        vga_clear_page(0, 0, SCREEN_SIZE);
        vga_clear_depth();

        if (g_state == STATE_PLAYING)
        {
            flevel_draw(&cam, &surf, &screen_rect);
            fentity_draw(&cam, &surf, &screen_rect);
        }
        ui_draw();

        vga_flip(surf.back);
    }
QUIT:
    cv_io_keyboard_shutdown();
    engine_shutdown();
    return 0;
}

// Unity Build includes
#include "src/fgame.c"
#include "src/fplayr.c"
#include "src/flevel.c"
#include "src/fentity.c"
#include "src/fui.c"
#include "src/faudio.c"
