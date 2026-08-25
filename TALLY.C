/*
 * CyberVGA SDK - The Tally - Burak Yazar
 */

#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include "CORE/CYBER.H"
#include "CORE/FIXED.H"
#include "CORE/PLATFRM.H"
#include "SOUND/BLAST.H"
#include "SOUND/SMIX.H"
#include "RNDR/GUI.H"
#include "RNDR/FONT.H"
#include "RNDR/SPRIT.H"
#include "CORE/TYPES/GFX.H"

#define SCREEN_W 320
#define SCREEN_H 240

/* Default sound config */
static BlastConfig g_blast_cfg = {
    0x220, 5, 1, 22050};

#include "src/game.h"
#include "src/player.h"
#include "src/level.h"
#include "src/entity.h"
#include "src/ui.h"
#include "src/audio.h"

static PCM8UFile m_menu, m_01, m_02, m_03, m_04, m_05, m_06, m_over, m_win;
static int menuplayed = 0, m_01played = 0, m_02played = 0, m_03played = 0, m_04played = 0, m_05played = 0, m_06played = 0, gameoverplayed = 0, winplayed = 0;

int main(int argc, char **argv)
{
    Camera cam;
    Surface8 surf = {SCREEN_W, SCREEN_H, 0, 0};
    int loopdone = 0;

    (void)argc;
    (void)argv;

#ifdef PLATFORM_WIN64
    platfrm_chdir_project_root();
#endif

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
            printf("Engine init failed\n");
            return 1;
        }
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
    }

    while (!loopdone)
    {
        engine_update(&cam);
        smix_update(&cam);

        if (cv_io_key_down(KEY_ESC))
        {
            loopdone = 1;
        }

        vga_clear_page(0, 0, SCREEN_SIZE);
        vga_clear_depth();

        vga_flip(surf.back);
    }
QUIT:
    engine_shutdown();
    return 0;
}

// Unity Build includes
#include "src/game.c"
#include "src/player.c"
#include "src/level.c"
#include "src/entity.c"
#include "src/ui.c"
#include "src/audio.c"
