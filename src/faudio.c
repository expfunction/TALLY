#include "faudio.h"
#include "fgame.h"
#include "fplayr.h"
#include "SOUND/BLAST.H"
#include "SOUND/SMIX.H"
#include "CORE/CONSL.H"
#include <stdio.h>
#include <string.h>

#define BGM_SLOT 0
#define STEP_SLOT 1
#define RADIO_SLOT 2
#define SFX_SLOT_START 3

int g_master_volume = 100;

static PCM8UFile s_music_title;
static int s_music_title_loaded = 0;

static PCM8UFile s_music_ambience;
static int s_music_ambience_loaded = 0;

static PCM8UFile s_sfx_gun;
static int s_sfx_gun_loaded = 0;

static PCM8UFile s_sfx_mldeth;
static int s_sfx_mldeth_loaded = 0;

static PCM8UFile s_sfx_test;
static int s_sfx_test_loaded = 0;

static PCM8UFile s_sfx_alarm;
static int s_sfx_alarm_loaded = 0;

static PCM8UFile s_sfx_door_open;
static int s_sfx_door_open_loaded = 0;

static PCM8UFile s_sfx_door_locked;
static int s_sfx_door_locked_loaded = 0;

static PCM8UFile s_sfx_pickup;
static int s_sfx_pickup_loaded = 0;

static PCM8UFile s_sfx_radio;
static int s_sfx_radio_loaded = 0;

static PCM8UFile s_sfx_step;
static int s_sfx_step_loaded = 0;

static PCM8UFile s_sfx_wallswap;
static int s_sfx_wallswap_loaded = 0;

static PCM8UFile s_sfx_fmdeth;
static int s_sfx_fmdeth_loaded = 0;

static PCM8UFile s_sfx_emptygun;
static int s_sfx_emptygun_loaded = 0;

static int s_next_sfx_slot = SFX_SLOT_START;

static int get_free_sfx_slot(void)
{
    int i;
    for (i = SFX_SLOT_START; i < MAX_SND_SRC; i++)
    {
        if (!g_snd_sources[i].active)
            return i;
    }
    int slot = s_next_sfx_slot;
    s_next_sfx_slot++;
    if (s_next_sfx_slot >= MAX_SND_SRC)
        s_next_sfx_slot = SFX_SLOT_START;
    return slot;
}

void audio_init(void)
{
    console_log("Loading audio assets from ASSTS\\SOUND...");

    s_music_title_loaded = blast_load_8u_pcm_file("ASSTS\\SOUND\\MAINOF.wav", &s_music_title);
    if (s_music_title_loaded)
        console_log("Loaded MAINOF.wav (Title Music, len: %u)", s_music_title.length);
    else
        console_log("Failed to load ASSTS\\SOUND\\MAINOF.wav");

    s_music_ambience_loaded = blast_load_8u_pcm_file("ASSTS\\SOUND\\AMBIENCE.wav", &s_music_ambience);
    if (s_music_ambience_loaded)
        console_log("Loaded AMBIENCE.wav (In-game Ambience, len: %u)", s_music_ambience.length);
    else
        console_log("Failed to load ASSTS\\SOUND\\AMBIENCE.wav");

    s_sfx_gun_loaded = blast_load_8u_pcm_file("ASSTS\\SOUND\\GUN.wav", &s_sfx_gun);
    if (s_sfx_gun_loaded)
        console_log("Loaded GUN.wav (Gun Fire, len: %u)", s_sfx_gun.length);
    else
        console_log("Failed to load ASSTS\\SOUND\\GUN.wav");

    s_sfx_mldeth_loaded = blast_load_8u_pcm_file("ASSTS\\SOUND\\MLDETH.wav", &s_sfx_mldeth);
    if (s_sfx_mldeth_loaded)
        console_log("Loaded MLDETH.wav (Male Death, len: %u)", s_sfx_mldeth.length);
    else
        console_log("Failed to load ASSTS\\SOUND\\MLDETH.wav");

    s_sfx_test_loaded = blast_load_8u_pcm_file("ASSTS\\SOUND\\TEST.wav", &s_sfx_test);
    if (s_sfx_test_loaded)
        console_log("Loaded TEST.wav (Test tone, len: %u)", s_sfx_test.length);

    s_sfx_alarm_loaded = blast_load_8u_pcm_file("ASSTS\\SOUND\\ALARM.wav", &s_sfx_alarm);
    if (s_sfx_alarm_loaded)
        console_log("Loaded ALARM.wav (Alarm SFX, len: %u)", s_sfx_alarm.length);

    s_sfx_door_open_loaded = blast_load_8u_pcm_file("ASSTS\\SOUND\\DOOR_OPN.wav", &s_sfx_door_open);
    if (s_sfx_door_open_loaded)
        console_log("Loaded DOOR_OPN.wav (Door Open SFX, len: %u)", s_sfx_door_open.length);

    s_sfx_door_locked_loaded = blast_load_8u_pcm_file("ASSTS\\SOUND\\DOOR_LCK.wav", &s_sfx_door_locked);
    if (s_sfx_door_locked_loaded)
        console_log("Loaded DOOR_LCK.wav (Door Locked SFX, len: %u)", s_sfx_door_locked.length);

    s_sfx_pickup_loaded = blast_load_8u_pcm_file("ASSTS\\SOUND\\PICKUP.wav", &s_sfx_pickup);
    if (s_sfx_pickup_loaded)
        console_log("Loaded PICKUP.wav (Pickup SFX, len: %u)", s_sfx_pickup.length);

    s_sfx_radio_loaded = blast_load_8u_pcm_file("ASSTS\\SOUND\\RADIO.wav", &s_sfx_radio);
    if (s_sfx_radio_loaded)
        console_log("Loaded RADIO.wav (Radio SFX, len: %u)", s_sfx_radio.length);

    s_sfx_step_loaded = blast_load_8u_pcm_file("ASSTS\\SOUND\\STEP.wav", &s_sfx_step);
    if (s_sfx_step_loaded)
        console_log("Loaded STEP.wav (Footstep SFX, len: %u)", s_sfx_step.length);

    s_sfx_wallswap_loaded = blast_load_8u_pcm_file("ASSTS\\SOUND\\WALLSWAP.wav", &s_sfx_wallswap);
    if (s_sfx_wallswap_loaded)
        console_log("Loaded WALLSWAP.wav (Wall Swap SFX, len: %u)", s_sfx_wallswap.length);

    s_sfx_fmdeth_loaded = blast_load_8u_pcm_file("ASSTS\\SOUND\\FMDETH.wav", &s_sfx_fmdeth);
    if (s_sfx_fmdeth_loaded)
        console_log("Loaded FMDETH.wav (Female Death SFX, len: %u)", s_sfx_fmdeth.length);

    s_sfx_emptygun_loaded = blast_load_8u_pcm_file("ASSTS\\SOUND\\EMPTYGUN.wav", &s_sfx_emptygun);
    if (s_sfx_emptygun_loaded)
        console_log("Loaded EMPTYGUN.wav (Dry Fire SFX, len: %u)", s_sfx_emptygun.length);
}

static PCM8UFile *get_sfx_pcm(SFXID id)
{
    switch (id)
    {
    case SFX_GUN_FIRE:
        return s_sfx_gun_loaded ? &s_sfx_gun : NULL;
    case SFX_MALE_DEATH:
        return s_sfx_mldeth_loaded ? &s_sfx_mldeth : NULL;
    case SFX_FEMALE_DEATH:
        return s_sfx_fmdeth_loaded ? &s_sfx_fmdeth : (s_sfx_mldeth_loaded ? &s_sfx_mldeth : NULL);
    case SFX_EMPTY_GUN:
        return s_sfx_emptygun_loaded ? &s_sfx_emptygun : (s_sfx_door_locked_loaded ? &s_sfx_door_locked : NULL);
    case SFX_HIT:
        return s_sfx_mldeth_loaded ? &s_sfx_mldeth : (s_sfx_test_loaded ? &s_sfx_test : NULL);
    case SFX_PICKUP:
        return s_sfx_pickup_loaded ? &s_sfx_pickup : (s_sfx_test_loaded ? &s_sfx_test : NULL);
    case SFX_DOOR_OPEN:
        return s_sfx_door_open_loaded ? &s_sfx_door_open : (s_sfx_test_loaded ? &s_sfx_test : NULL);
    case SFX_DOOR_LOCKED:
        return s_sfx_door_locked_loaded ? &s_sfx_door_locked : (s_sfx_test_loaded ? &s_sfx_test : NULL);
    case SFX_TERMINAL:
        return s_sfx_door_locked_loaded ? &s_sfx_door_locked : (s_sfx_radio_loaded ? &s_sfx_radio : NULL);
    case SFX_WALL_SWAP:
        return s_sfx_wallswap_loaded ? &s_sfx_wallswap : (s_sfx_test_loaded ? &s_sfx_test : NULL);
    case SFX_ALARM:
        return s_sfx_alarm_loaded ? &s_sfx_alarm : (s_sfx_test_loaded ? &s_sfx_test : NULL);
    case SFX_RADIO_CLICK:
        return s_sfx_radio_loaded ? &s_sfx_radio : (s_sfx_test_loaded ? &s_sfx_test : NULL);
    case SFX_STEP:
        return s_sfx_step_loaded ? &s_sfx_step : NULL;
    case SFX_ENDING_STING:
        return s_sfx_wallswap_loaded ? &s_sfx_wallswap : (s_sfx_test_loaded ? &s_sfx_test : NULL);
    case SFX_TEST:
    default:
        return s_sfx_test_loaded ? &s_sfx_test : NULL;
    }
}

void audio_play_title_music(void)
{
    Vec4 pos;
    if (!s_music_title_loaded || !s_music_title.data)
        return;

    if (g_snd_sources[BGM_SLOT].active && g_snd_sources[BGM_SLOT].pcm == &s_music_title)
    {
        g_snd_sources[BGM_SLOT].looping = true;
        return; /* Already playing and looping */
    }

    i32 vol = (FX_ONE * g_master_volume) / 100;
    pos = g_player.w_pos;
    smix_play(BGM_SLOT, &s_music_title, &pos, vol, FX_ONE * 1000, FX_ONE * 2000, true);
    g_snd_sources[BGM_SLOT].looping = true;
    console_log("Title music started looping (MAINOF.wav)");
}

void audio_stop_title_music(void)
{
    if (g_snd_sources[BGM_SLOT].active && g_snd_sources[BGM_SLOT].pcm == &s_music_title)
    {
        smix_stop(BGM_SLOT);
        console_log("Title music stopped");
    }
}

void audio_play_ambience(void)
{
    Vec4 pos;
    if (!s_music_ambience_loaded || !s_music_ambience.data)
        return;

    if (g_snd_sources[BGM_SLOT].active && g_snd_sources[BGM_SLOT].pcm == &s_music_ambience)
    {
        g_snd_sources[BGM_SLOT].looping = true;
        return; /* Already playing and looping */
    }

    i32 vol = (FX_ONE * g_master_volume) / 100;
    pos = g_player.w_pos;
    smix_play(BGM_SLOT, &s_music_ambience, &pos, vol, FX_ONE * 1000, FX_ONE * 2000, true);
    g_snd_sources[BGM_SLOT].looping = true;
    console_log("Ambience background music started looping (AMBIENCE.wav)");
}

void audio_stop_ambience(void)
{
    if (g_snd_sources[BGM_SLOT].active && g_snd_sources[BGM_SLOT].pcm == &s_music_ambience)
    {
        smix_stop(BGM_SLOT);
        console_log("Ambience stopped");
    }
}

void audio_play_step(void)
{
    Vec4 pos;
    if (!s_sfx_step_loaded || !s_sfx_step.data)
        return;

    // Cut previous step sound immediately on dedicated slot
    smix_stop(STEP_SLOT);

    i32 vol = (FX_FROM_FLOAT(0.5f) * g_master_volume) / 100;
    pos = g_player.w_pos;
    smix_play(STEP_SLOT, &s_sfx_step, &pos, vol, FX_ONE * 1000, FX_ONE * 2000, false);
}

void audio_stop_step(void)
{
    if (g_snd_sources[STEP_SLOT].active && g_snd_sources[STEP_SLOT].pcm == &s_sfx_step)
    {
        smix_stop(STEP_SLOT);
    }
}

void audio_set_step_position(const Vec4 *pos)
{
    if (g_snd_sources[STEP_SLOT].active)
    {
        smix_set_position(STEP_SLOT, pos ? pos : &g_player.w_pos);
    }
}

void audio_stop_all(void)
{
    smix_stop_all();
}

void audio_play_sfx(SFXID id)
{
    if (id == SFX_STEP)
    {
        audio_play_step();
        return;
    }

    PCM8UFile *pcm = get_sfx_pcm(id);
    int slot;
    Vec4 pos;

    if (!pcm || !pcm->data)
        return;

    slot = get_free_sfx_slot();
    pos = g_player.w_pos;
    i32 vol = (FX_ONE * g_master_volume) / 100;
    smix_play(slot, pcm, &pos, vol, FX_ONE * 1000, FX_ONE * 2000, false);
    console_log("Audio SFX %d played on slot %d", (int)id, slot);
}

void audio_play_sfx_at(SFXID id, const Vec4 *pos)
{
    PCM8UFile *pcm = get_sfx_pcm(id);
    int slot;
    const Vec4 *play_pos;

    if (!pcm || !pcm->data)
        return;

    slot = get_free_sfx_slot();
    play_pos = pos ? pos : &g_player.w_pos;
    i32 vol = (FX_ONE * g_master_volume) / 100;
    smix_play(slot, pcm, play_pos, vol, FX_ONE * 2, FX_ONE * 25, false);
    console_log("Audio 3D SFX %d played on slot %d", (int)id, slot);
}

int audio_is_radio_playing(void)
{
    return (g_snd_sources[RADIO_SLOT].active && g_snd_sources[RADIO_SLOT].pcm == &s_sfx_radio);
}

void audio_play_radio(int msg_id, const Vec4 *pos)
{
    const Vec4 *play_pos;
    console_log("Radio Msg %d: Broadcast tuned.", msg_id);

    if (!s_sfx_radio_loaded || !s_sfx_radio.data)
        return;

    // Only play if not already playing
    if (g_snd_sources[RADIO_SLOT].active && g_snd_sources[RADIO_SLOT].pcm == &s_sfx_radio)
    {
        console_log("Radio already active, ignoring re-trigger.");
        return;
    }

    play_pos = pos ? pos : &g_player.w_pos;
    // Play on dedicated RADIO_SLOT, non-looping
    i32 vol = (FX_ONE * g_master_volume) / 100;
    smix_play(RADIO_SLOT, &s_sfx_radio, play_pos, vol, FX_TWO, FX_FROM_INT(10), false);
    g_snd_sources[RADIO_SLOT].looping = false;
}

void audio_stop_radio(void)
{
    if (g_snd_sources[RADIO_SLOT].active && g_snd_sources[RADIO_SLOT].pcm == &s_sfx_radio)
    {
        smix_stop(RADIO_SLOT);
    }
}

void audio_set_master_volume(int vol)
{
    if (vol < 0)
        vol = 0;
    if (vol > 100)
        vol = 100;
    g_master_volume = vol;

    if (g_snd_sources[BGM_SLOT].active)
    {
        i32 bvol = (FX_ONE * g_master_volume) / 100;
        smix_set_params(BGM_SLOT, bvol, FX_ONE * 1000, FX_ONE * 2000, true);
    }
}

void audio_update(void)
{
    /* Keep BGM and Step sound positions synced with listener */
    if (g_snd_sources[BGM_SLOT].active)
    {
        smix_set_position(BGM_SLOT, &g_player.w_pos);
        g_snd_sources[BGM_SLOT].looping = true;
    }
    if (g_snd_sources[STEP_SLOT].active)
    {
        smix_set_position(STEP_SLOT, &g_player.w_pos);
    }

    /* Automatically sync title music and ambient background with game state */
    if (g_state == STATE_TITLE || g_state == STATE_HELP || (g_state == STATE_SETTINGS && g_previous_state == STATE_TITLE))
    {
        if (!g_snd_sources[BGM_SLOT].active || g_snd_sources[BGM_SLOT].pcm != &s_music_title)
        {
            audio_play_title_music();
        }
    }
    else if (g_state == STATE_PLAYING || g_state == STATE_MENU || (g_state == STATE_SETTINGS && g_previous_state == STATE_MENU))
    {
        if (s_music_ambience_loaded && s_music_ambience.data)
        {
            if (!g_snd_sources[BGM_SLOT].active || g_snd_sources[BGM_SLOT].pcm != &s_music_ambience)
            {
                audio_play_ambience();
            }
        }
        else
        {
            if (g_snd_sources[BGM_SLOT].active && g_snd_sources[BGM_SLOT].pcm == &s_music_title)
            {
                audio_stop_title_music();
            }
        }
    }
    else if (g_state == STATE_GAMEOVER || g_state == STATE_WIN)
    {
        if (g_snd_sources[BGM_SLOT].active && g_snd_sources[BGM_SLOT].pcm == &s_music_ambience)
        {
            audio_stop_ambience();
        }
    }
}
