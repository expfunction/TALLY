/* Copyright (c) 2026 Burak Yazar */

#ifndef FUI_H
#define FUI_H

#include "CORE/CYBER.H"

extern int unit4_betrayal_frame;

void ui_init(void);
void ui_update(void);
void ui_draw(void);

void ui_trigger_damage_flash(void);
void ui_trigger_muzzle_flash(void);
void ui_accelerate_credits(float delta);

void ui_set_subtitle(const char *speaker, const char *line1, const char *line2, int duration_frames, u8 color);
void ui_set_prompt(const char *prompt_text);

#endif
