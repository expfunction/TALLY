/* Copyright (c) 2026 Burak Yazar */

#include "faudio.h"

void audio_init(void)
{
    // Load SFX and Voice lines
}

void audio_play_sfx(int id)
{
    // Trigger punchy SFX
    console_log("SFX %d played", id);
}

void audio_play_radio(int msg_id)
{
    // Play warm Squealer radio voice
    console_log("Radio Msg %d: Trust the Party.", msg_id);
}

void audio_update(void)
{
    // Update audio listeners and falloff
}
