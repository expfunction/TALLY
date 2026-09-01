/* Copyright (c) 2026 Burak Yazar */

#include "fsave.h"
#include "fgame.h"
#include "CORE/CONSL.H"
#include <stdio.h>
#include <string.h>

#define SAVE_MAGIC 0x54414C59 /* "TALY" */
#define SAVE_FILENAME "TALLYSAV.CSG"

typedef struct
{
    int magic;
    int version;
    int current_level;
    int health;
    int ammo;
    int loyalty;
    int doubt;
    int boxer_dead;
    int record_kept;
    int access_card_given;
    int assume_control;
    int mission_progress;
} TallySaveData;

static int s_cached_save_exists = -1;

void fsave_invalidate_cache(void)
{
    s_cached_save_exists = -1;
}

int fsave_checkpoint(void)
{
    FILE *fp;
    TallySaveData data;

    data.magic = SAVE_MAGIC;
    data.version = 1;
    data.current_level = (int)g_current_level;
    data.health = g_health;
    data.ammo = g_ammo;
    data.loyalty = g_loyalty;
    data.doubt = g_doubt;
    data.boxer_dead = g_boxer_dead;
    data.record_kept = g_record_kept;
    data.access_card_given = g_access_card_given;
    data.assume_control = g_assume_control;
    data.mission_progress = g_mission_progress;

    fp = fopen(SAVE_FILENAME, "wb");
    if (!fp)
    {
        console_log("Failed to write checkpoint save file");
        return 0;
    }

    if (fwrite(&data, sizeof(TallySaveData), 1, fp) != 1)
    {
        fclose(fp);
        return 0;
    }

    fclose(fp);
    s_cached_save_exists = 1;
    console_log("Autosave checkpoint written for Level %d", (int)g_current_level);
    return 1;
}

int fsave_exists(void)
{
    if (s_cached_save_exists != -1)
        return s_cached_save_exists;

    FILE *fp = fopen(SAVE_FILENAME, "rb");
    if (!fp)
    {
        s_cached_save_exists = 0;
        return 0;
    }

    TallySaveData data;
    if (fread(&data, sizeof(TallySaveData), 1, fp) != 1)
    {
        fclose(fp);
        s_cached_save_exists = 0;
        return 0;
    }
    fclose(fp);

    s_cached_save_exists = (data.magic == SAVE_MAGIC) ? 1 : 0;
    return s_cached_save_exists;
}

LevelID fsave_get_saved_level(void)
{
    FILE *fp = fopen(SAVE_FILENAME, "rb");
    if (!fp)
        return LEVEL_BARRACKS;

    TallySaveData data;
    if (fread(&data, sizeof(TallySaveData), 1, fp) != 1)
    {
        fclose(fp);
        return LEVEL_BARRACKS;
    }
    fclose(fp);

    if (data.magic != SAVE_MAGIC)
        return LEVEL_BARRACKS;

    return (LevelID)data.current_level;
}

int fsave_load(void)
{
    FILE *fp = fopen(SAVE_FILENAME, "rb");
    if (!fp)
        return 0;

    TallySaveData data;
    if (fread(&data, sizeof(TallySaveData), 1, fp) != 1)
    {
        fclose(fp);
        return 0;
    }
    fclose(fp);

    if (data.magic != SAVE_MAGIC)
        return 0;

    g_current_level = (LevelID)data.current_level;
    g_health = data.health;
    if (g_health <= 0)
        g_health = 100;
    g_ammo = data.ammo;
    if (g_ammo < 5)
        g_ammo = 5; // Guarantee at least 5 rounds on restart
    g_loyalty = data.loyalty;
    g_doubt = data.doubt;
    g_boxer_dead = data.boxer_dead;
    g_record_kept = data.record_kept;
    g_access_card_given = data.access_card_given;
    g_assume_control = data.assume_control;
    g_mission_progress = data.mission_progress;

    s_cached_save_exists = 1;
    console_log("Checkpoint loaded successfully. Level: %d, Progress: %d", (int)g_current_level, g_mission_progress);
    return 1;
}

void fsave_delete(void)
{
    remove(SAVE_FILENAME);
    s_cached_save_exists = 0;
}
 
