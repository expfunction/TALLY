/* Copyright (c) 2026 Burak Yazar */

#ifndef FAUDIO_H
#define FAUDIO_H

#include "CORE/CYBER.H"

typedef enum
{
    SFX_GUN_FIRE = 0,
    SFX_PISTOL_FIRE = 0,
    SFX_MALE_DEATH,
    SFX_FEMALE_DEATH,
    SFX_EMPTY_GUN,
    SFX_DRY_FIRE = SFX_EMPTY_GUN,
    SFX_TEST,
    SFX_PICKUP,
    SFX_DOOR_OPEN,
    SFX_DOOR_LOCKED,
    SFX_TERMINAL,
    SFX_WALL_SWAP,
    SFX_ALARM,
    SFX_RADIO_CLICK,
    SFX_HIT,
    SFX_STEP,
    SFX_ENDING_STING,
    SFX_MAX_COUNT
} SFXID;

extern int g_master_volume;

void audio_init(void);
void audio_play_sfx(SFXID id);
void audio_play_sfx_at(SFXID id, const Vec4 *pos);
void audio_play_title_music(void);
void audio_stop_title_music(void);
void audio_play_ambience(void);
void audio_stop_ambience(void);
void audio_play_step(void);
void audio_stop_step(void);
void audio_set_step_position(const Vec4 *pos);
void audio_stop_all(void);
int audio_is_radio_playing(void);
void audio_play_radio(int msg_id, const Vec4 *pos);
void audio_stop_radio(void);
void audio_set_master_volume(int vol);
void audio_update(void);

#endif
