/* Copyright (c) 2026 Burak Yazar */

/* HUD layout for CyberVGA — 320x240 Mode X bottom bar */
#ifndef HUD_H
#define HUD_H

#include "../CORE/TYPES/GFX.H"
#include "../RNDR/SPRIT.h"
#include "PLAYER.H"

/* ---- Screen / HUD geometry (all in pixels) ---- */

/* Game viewport now covers full screen (no HUD_Y0 clipping) */

/* Portrait / Top-Left Anchor */
#define HUD_PORT_X0 5
#define HUD_PORT_Y0 5
#define HUD_PORT_X1 73
#define HUD_PORT_Y1 73

/* Health / stamina bar (top left) */
#define HUD_BAR_X0 (HUD_PORT_X1 + 5)
#define HUD_BAR_Y0 5
#define HUD_BAR_X1 (HUD_BAR_X0 + 100)
#define HUD_BAR_Y1 14

/* Quickslot dimensions */
#define HUD_SLOT_SIZE 30

/* Q slot (below portrait) */
#define HUD_SLOTQ_X0 5
#define HUD_SLOTQ_Y0 (HUD_PORT_Y1 + 5)

/* E slot (right of Q slot) */
#define HUD_SLOTE_X0 (HUD_SLOTQ_X0 + HUD_SLOT_SIZE + 5)
#define HUD_SLOTE_Y0 (HUD_PORT_Y1 + 5)

/* Echo Bead (bottom right mini slot) */
#define HUD_BEAD_X0 (320 - HUD_SLOT_SIZE - 5)
#define HUD_BEAD_Y0 (240 - HUD_SLOT_SIZE - 5)

/* Dialogue text box (bottom center) */
#define HUD_DIAL_X0 20
#define HUD_DIAL_Y0 180
#define HUD_DIAL_X1 300
#define HUD_DIAL_Y1 230

/* ---- Palette colour slots (tune to HRMNAT.PAL) ---- */
#define HUD_COL_BG 0      /* bar background (black) */
#define HUD_COL_BORDER 95 /* gold/brown frame lines */
#define HUD_COL_SLOT_BG 1 /* slot / box fill */
#define HUD_COL_HEALTH 40 /* health bar fill */
#define HUD_COL_TEXT 255  /* dialogue text */

/**
 * @brief Draw the full HUD for one frame.
 * @param dst      VGA back-buffer pointer (from vga_backbuffer())
 * @param pl       Current player state
 * @param portrait Portrait sprite (may be NULL)
 * @param q_icon   Q slot icon (may be NULL)
 * @param e_icon   E slot icon (may be NULL)
 * @param bead     Echo bead icon (may be NULL)
 * @param cooldown_q Cooldown remaining for Q (0.0 to 1.0)
 * @param cooldown_e Cooldown remaining for E (0.0 to 1.0)
 */
void hud_draw(u8 *dst, const Player *pl, const Sprite *portrait,
              const Sprite *q_icon, const Sprite *e_icon, const Sprite *bead,
              i32 cooldown_q, i32 cooldown_e);

#endif /* HUD_H */
