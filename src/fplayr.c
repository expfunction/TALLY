#include "fplayr.h"
#include "IO/IO.H"
#include "RNDR/CAMER.H"
#include "PHYS/PHYS.H"
#include "CORE/CYBER.H"
#include "fai.h"
#include "fui.h"
#include "faudio.h"
#include "fgame.h"
#include "flevel.h"
#include "fentity.h"

FPlayer g_player;
i32 g_mouse_sensitivity = FX_ONE;

void fplayer_init(void)
{
    g_player.w_pos.x = 0;
    g_player.w_pos.y = FX_ONE; // Start a bit above ground
    g_player.w_pos.z = 0;
    g_player.w_pos.w = FX_ONE;
    g_player.w_rot.x = 0;     // pitch (level horizon)
    g_player.w_rot.y = FX_PI; // yaw (facing North toward exit)
}

void fplayer_set_start_pos_rot(Vec4 pos, i32 yaw, i32 pitch)
{
    g_player.w_pos = pos;
    g_player.w_pos.y = FX_ONE;
    g_player.w_rot.x = pitch;
    g_player.w_rot.y = yaw;
}

void fplayer_set_start_pos(Vec4 pos)
{
    fplayer_set_start_pos_rot(pos, FX_PI, 0);
}

void fplayer_sync_camera(Camera *cam)
{
    if (!cam)
        return;
    Vec3 p = {g_player.w_pos.x, g_player.w_pos.y, g_player.w_pos.z};
    Vec3 r = {0, g_player.w_rot.y, 0};
    cam_init(cam, &p, &r, CAM_PROJ_PERSPECTIVE);
    phys_collide_camera(cam, &g_world);
    g_player.w_pos = cam->position;
    audio_set_step_position(&g_player.w_pos);
}

void fplayer_update(Camera *cam)
{
    //  Mouse look
    MouseIO mouse;
    if (cv_io_mouse_state(&mouse))
    {
        // dx and dy are integer screen deltas
        i32 delta_yaw = fx_mul_q16(FX_FROM_INT(mouse.dx), g_mouse_sensitivity) / 100;
        g_player.w_rot.y += delta_yaw; // Yaw
        g_player.w_rot.x = 0;
    }

    //  WASD Movement based on yaw
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

    // Normalize diagonal movement vectors
    if (dx != 0 && dz != 0)
    {
        dx = fx_mul_q16(dx, FX_FROM_FLOAT(0.70710678f));
        dz = fx_mul_q16(dz, FX_FROM_FLOAT(0.70710678f));
    }

    static i32 step_accum = 0;
    const i32 step_threshold = FX_FROM_FLOAT(0.38f);

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

        if (step_accum == 0)
        {
            audio_play_step();
        }

        step_accum += g_clock.dt;
        if (step_accum >= step_threshold)
        {
            audio_play_step(); // Cuts previous step sound and restarts
            step_accum = FX_ONE / 1000;
        }
    }
    else
    {
        if (step_accum != 0)
        {
            audio_stop_step();
            step_accum = 0;
        }
    }

    // Hook camera
    Vec3 p = {g_player.w_pos.x, g_player.w_pos.y, g_player.w_pos.z};
    Vec3 r = {0, g_player.w_rot.y, 0};
    cam_init(cam, &p, &r, CAM_PROJ_PERSPECTIVE);

    // Optional: Collide camera against the world walls using physics system
    phys_collide_camera(cam, &g_world);
    g_player.w_pos = cam->position; // Update back player position if pushed

    // Check collision against closed doors (2D horizontal distance)
    FEntity *ents = fentity_get_all();
    for (int i = 0; i < 128; i++)
    {
        if (ents[i].active && (ents[i].type == ENT_TYPE_DOOR || ents[i].type == ENT_TYPE_DOOR_LOCKED))
        {
            if (ents[i].state == STATE_CLOSED || ents[i].pos.y < FX_FROM_FLOAT(1.4f))
            {
                Vec4 diff;
                diff.x = g_player.w_pos.x - ents[i].pos.x;
                diff.y = 0;
                diff.z = g_player.w_pos.z - ents[i].pos.z;
                diff.w = 0;
                i32 dist = vec4_length(&diff);
                i32 door_radius = FX_FROM_FLOAT(1.2f); // Door is 2x2, so radius ~1.0 + player radius
                if (dist < door_radius && dist > 0)
                {
                    Vec4 push_dir = diff;
                    vec4_normalize3(&push_dir, &push_dir);

                    i32 overlap = door_radius - dist;
                    g_player.w_pos.x += fx_mul_q16(push_dir.x, overlap);
                    g_player.w_pos.z += fx_mul_q16(push_dir.z, overlap);
                    cam->position = g_player.w_pos;
                }
            }
        }
    }

    // Check collision against character entities (prevent clipping through enemies/NPCs)
    for (int i = 0; i < 128; i++)
    {
        if (ents[i].active && (ents[i].type == ENT_TYPE_ENFORCER_F || ents[i].type == ENT_TYPE_ENFORCER_M ||
                               ents[i].type == ENT_TYPE_UNIT4 || ents[i].type == ENT_TYPE_BOXER ||
                               ents[i].type == ENT_TYPE_WORKER))
        {
            Vec4 diff;
            diff.x = g_player.w_pos.x - ents[i].pos.x;
            diff.y = 0;
            diff.z = g_player.w_pos.z - ents[i].pos.z;
            diff.w = 0;
            i32 dist = vec4_length(&diff);
            i32 char_radius = FX_FROM_FLOAT(0.85f);
            if (dist < char_radius && dist > 0)
            {
                Vec4 push_dir = diff;
                vec4_normalize3(&push_dir, &push_dir);
                i32 overlap = char_radius - dist;
                g_player.w_pos.x += fx_mul_q16(push_dir.x, overlap);
                g_player.w_pos.z += fx_mul_q16(push_dir.z, overlap);
                cam->position = g_player.w_pos;
            }
        }
    }

    // Keep footstep emitter locked to player position
    audio_set_step_position(&g_player.w_pos);

    // Proximity prompt scanning (find closest interactable entity in 2D)
    FEntity *all_ents = fentity_get_all();
    FEntity *closest_ent = NULL;
    i32 min_interactive_dist = FX_FROM_FLOAT(3.0f);

    for (int i = 0; i < 128; i++)
    {
        if (!all_ents[i].active)
            continue;

        Vec4 d;
        d.x = g_player.w_pos.x - all_ents[i].pos.x;
        d.y = 0;
        d.z = g_player.w_pos.z - all_ents[i].pos.z;
        d.w = 0;
        i32 dist = vec4_length(&d);
        if (dist < min_interactive_dist)
        {
            if (all_ents[i].type == ENT_TYPE_DOOR ||
                all_ents[i].type == ENT_TYPE_DOOR_LOCKED ||
                all_ents[i].type == ENT_TYPE_EXTRACTION ||
                all_ents[i].type == ENT_TYPE_LEDGER ||
                all_ents[i].type == ENT_TYPE_RADIO ||
                all_ents[i].type == ENT_TYPE_BOXER ||
                all_ents[i].type == ENT_TYPE_TERMINAL ||
                all_ents[i].type == ENT_TYPE_VAN ||
                all_ents[i].type == ENT_TYPE_RATHOLE)
            {
                min_interactive_dist = dist;
                closest_ent = &all_ents[i];
            }
        }
    }

    if (closest_ent)
    {
        switch (closest_ent->type)
        {
        case ENT_TYPE_DOOR:
            if (closest_ent->state == STATE_CLOSED || closest_ent->state == STATE_CLOSING)
                ui_set_prompt("[E] OPEN DOOR");
            break;
        case ENT_TYPE_DOOR_LOCKED:
            ui_set_prompt("[LOCKED - CLEARANCE REQUIRED]");
            break;
        case ENT_TYPE_EXTRACTION:
            if (g_current_level == LEVEL_BARRACKS)
            {
                if (g_mission_progress == 1)
                    ui_set_prompt("[E] DEPLOY TO RATION BLOCK");
                else
                    ui_set_prompt("[E] DEPLOY TO GENERATOR");
            }
            else if (g_current_level == LEVEL_RATIONBLOCK)
            {
                ui_set_prompt("[E] RETURN TO BARRACKS");
            }
            else if (g_current_level == LEVEL_GENERATOR)
            {
                ui_set_prompt("[E] COMPLETE MISSION");
            }
            break;
        case ENT_TYPE_LEDGER:
            ui_set_prompt("[E] PRESERVE RATION LEDGER");
            break;
        case ENT_TYPE_RADIO:
            if (audio_is_radio_playing())
                ui_set_prompt("[DIRECTORATE BROADCAST ACTIVE]");
            else
                ui_set_prompt("[E] TUNE DIRECTORATE BROADCAST");
            break;
        case ENT_TYPE_BOXER:
            if (g_record_kept && !g_access_card_given)
                ui_set_prompt("[E] SURRENDER ACCESS CARD");
            else if (g_access_card_given)
                ui_set_prompt("[CLEARANCE REVOKED: LABOURER ESCAPING]");
            else
                ui_set_prompt("[FORCED LABOURER: NO EVIDENCE HELD]");
            break;
        case ENT_TYPE_TERMINAL:
            if (g_record_kept && !g_access_card_given)
                ui_set_prompt("[E] CLAIM CONTINUITY TERMINAL");
            else
                ui_set_prompt("[CONTINUITY TERMINAL: STANDBY]");
            break;
        case ENT_TYPE_VAN:
            if (!g_record_kept && !g_access_card_given)
                ui_set_prompt("[E] SECURE DETAINEES (FINISH MISSION)");
            else if (g_assume_control)
                ui_set_prompt("[E] SEIZE DIRECTORATE VAN");
            else
                ui_set_prompt("[RECYCLING VAN]");
            break;
        case ENT_TYPE_RATHOLE:
            if (g_access_card_given)
                ui_set_prompt("[E] ESCAPE VIA SERVICE TUNNEL");
            else
                ui_set_prompt("[SERVICE TUNNEL]");
            break;
        default:
            break;
        }
    }

    // Interaction check (manual debounce with cooldown to prevent spamming)
    static int e_cooldown = 0;
    if (e_cooldown > 0)
        e_cooldown--;

    int e_is_down = cv_io_key_down(KEY_E);

    if (e_is_down && e_cooldown == 0 && closest_ent)
    {
        e_cooldown = 20; // 20 frames cooldown (~0.33s at 60fps)
        FEntity *ent = closest_ent;

        if (ent->type == ENT_TYPE_EXTRACTION)
        {
            audio_play_sfx(SFX_DOOR_OPEN);
            if (g_current_level == LEVEL_BARRACKS)
            {
                if (g_mission_progress == 1)
                {
                    fgame_load_level(LEVEL_RATIONBLOCK);
                    ui_set_subtitle("DIRECTORATE BROADCAST", "Arrived at Ration Block. Correct the ledger discrepancy.", "Silence all unauthorized worker assembly.", 540, 43);
                }
                else if (g_mission_progress >= 2)
                {
                    fgame_load_level(LEVEL_GENERATOR);
                    ui_set_subtitle("DIRECTORATE BROADCAST", "Arrived at Generator Core. Saboteur identified.", "Detain the laborer immediately.", 540, 43);
                }
                else
                {
                    g_state = STATE_WIN;
                }
            }
            else if (g_current_level == LEVEL_RATIONBLOCK)
            {
                g_mission_progress = 2;
                fgame_load_level(LEVEL_BARRACKS);
                ui_set_subtitle("DIRECTORATE BROADCAST", "Compliance report received. Return to Barracks.", "The civic record is restored.", 540, 43);
            }
            else if (g_current_level == LEVEL_GENERATOR)
            {
                audio_play_sfx(SFX_ENDING_STING);
                if (g_record_kept && g_access_card_given)
                {
                    g_ending = 2; // B: Untrustworthy
                }
                else if (g_record_kept && g_assume_control)
                {
                    g_ending = 3; // C: Successor
                }
                else
                {
                    g_ending = 1; // A: Reliable
                }
                g_state = STATE_WIN;
            }
            if (g_state == STATE_PLAYING)
            {
                fplayer_sync_camera(cam);
            }
        }
        else if (ent->type == ENT_TYPE_DOOR)
        {
            if (ent->state == STATE_CLOSED || ent->state == STATE_CLOSING)
            {
                ent->state = STATE_OPENING;
                audio_play_sfx_at(SFX_DOOR_OPEN, &ent->pos);
                fai_set_obstacle((int)(FX_TO_FLOAT(ent->pos.x) / 2.0f + 0.5f),
                                 (int)(FX_TO_FLOAT(ent->pos.z) / 2.0f + 0.5f), 0);
            }
        }
        else if (ent->type == ENT_TYPE_DOOR_LOCKED)
        {
            audio_play_sfx(SFX_DOOR_LOCKED);
            ui_set_subtitle("SECURITY SYSTEM", "Access Denied: High-level Directorate clearance required.", "", 240, 40);
        }
        else if (ent->type == ENT_TYPE_LEDGER)
        {
            g_record_kept = 1;
            g_doubt += 1;
            ent->active = 0;
            audio_play_sfx(SFX_PICKUP);
            ui_set_subtitle("RECORD PRESERVED", "Physical ration ledger acquired.", "Figures prove workers were deprived. Quota was falsified.", 600, 31);
        }
        else if (ent->type == ENT_TYPE_RADIO)
        {
            if (!audio_is_radio_playing())
            {
                g_loyalty += 1;
                audio_play_radio(1, &ent->pos);
            }
            ui_set_subtitle("DIRECTORATE BROADCAST", "All quotas in Sector 9 are met. All citizens are counted.", "Do not heed false rumors. Trust the Directorate.", 600, 43);
        }
        else if (ent->type == ENT_TYPE_BOXER)
        {
            if (g_record_kept && !g_access_card_given)
            {
                g_access_card_given = 1;
                g_doubt += 2;
                audio_play_sfx(SFX_ALARM);
                ui_set_subtitle("DIRECTORATE ALARM", "CLEARANCE REVOKED: Unit 9 marked as hostile breach!", "Unit 4 dispatched to neutralize traitor.", 600, 40);

                FEntity *all = fentity_get_all();
                for (int u = 0; u < 128; u++)
                {
                    if (all[u].active && all[u].type == ENT_TYPE_UNIT4)
                    {
                        all[u].state = STATE_BETRAYAL;
                    }
                }
            }
            else if (!g_record_kept)
            {
                ui_set_subtitle("FORCED LABOURER", "\"Without the physical ledger, there is nothing to transmit...\"", "\"The Directorate has already rewritten our history.\"", 600, 31);
            }
        }
        else if (ent->type == ENT_TYPE_TERMINAL)
        {
            if (g_record_kept && !g_access_card_given)
            {
                g_assume_control = 1;
                g_loyalty += 2;
                audio_play_sfx(SFX_TERMINAL);
                ui_set_subtitle("SYSTEM CONTINUITY PROTOCOL", "Override accepted: Unit 9 registered as New Authority.", "Leadership broadcast feed severed. Assume Control.", 600, 43);
            }
            else
            {
                ui_set_subtitle("CONTINUITY TERMINAL", "Terminal locked. Evidence leverage required for takeover.", "", 300, 43);
            }
        }
        else if (ent->type == ENT_TYPE_VAN)
        {
            audio_play_sfx(SFX_ENDING_STING);
            if (!g_record_kept && !g_access_card_given)
            {
                g_ending = 1; // Reliable
                g_state = STATE_WIN;
            }
            else if (g_assume_control)
            {
                g_ending = 3; // Successor
                g_state = STATE_WIN;
            }
        }
        else if (ent->type == ENT_TYPE_RATHOLE)
        {
            audio_play_sfx(SFX_ENDING_STING);
            if (g_access_card_given)
            {
                g_ending = 2; // Untrustworthy
                g_state = STATE_WIN;
            }
            else if (g_assume_control)
            {
                g_ending = 3;
                g_state = STATE_WIN;
            }
        }
    }

    // Shooting mechanics (LMB - Instant Raycasting through level grid)
    static int prev_lmb = 0;
    if (mouse.btn_left && !prev_lmb)
    {
        if (g_ammo > 0)
        {
            g_ammo--;
            audio_play_sfx(SFX_PISTOL_FIRE);
            ui_trigger_muzzle_flash();

            // Compute ray origin and forward direction
            Vec4 curr_pos = g_player.w_pos;

            i32 s_y = fx_sin(g_player.w_rot.y);
            i32 c_y = fx_cos(g_player.w_rot.y);
            i32 s_x = fx_sin(g_player.w_rot.x);
            i32 c_x = fx_cos(g_player.w_rot.x);

            Vec4 ray_dir;
            ray_dir.x = fx_mul_q16(c_x, s_y);
            ray_dir.y = -s_x;
            ray_dir.z = fx_mul_q16(c_x, c_y);
            ray_dir.w = 0;

            const i32 step_size = FX_FROM_FLOAT(0.2f);
            const int max_steps = 150; // 30 world units max range

            FEntity *all_ents = fentity_get_all();

            for (int step = 1; step <= max_steps; step++)
            {
                curr_pos.x += fx_mul_q16(ray_dir.x, step_size);
                curr_pos.y += fx_mul_q16(ray_dir.y, step_size);
                curr_pos.z += fx_mul_q16(ray_dir.z, step_size);

                //  Check solid level grid obstacles
                int gx = (int)(FX_TO_FLOAT(curr_pos.x) / 2.0f + 0.5f);
                int gz = (int)(FX_TO_FLOAT(curr_pos.z) / 2.0f + 0.5f);

                if (gx < 0 || gx >= MAP_WIDTH || gz < 0 || gz >= MAP_HEIGHT)
                {
                    break; // Out of bounds
                }

                if (g_map_grid[gz * MAP_WIDTH + gx])
                {
                    break; // Hit wall
                }

                // Check closed doors
                int hit_door = 0;
                for (int j = 0; j < 128; j++)
                {
                    if (!all_ents[j].active)
                        continue;
                    FEntity *door = &all_ents[j];
                    if (door->type == ENT_TYPE_DOOR || door->type == ENT_TYPE_DOOR_LOCKED)
                    {
                        if (door->state == STATE_CLOSED || door->state == STATE_CLOSING || door->pos.y < FX_FROM_FLOAT(1.4f))
                        {
                            Vec4 d_diff;
                            d_diff.x = curr_pos.x - door->pos.x;
                            d_diff.y = 0;
                            d_diff.z = curr_pos.z - door->pos.z;
                            d_diff.w = 0;
                            if (vec4_length(&d_diff) < FX_FROM_FLOAT(1.1f))
                            {
                                hit_door = 1;
                                break;
                            }
                        }
                    }
                }
                if (hit_door)
                {
                    break; // Ray blocked by closed door
                }

                //  Check shootable character entities
                int hit_entity = 0;
                for (int j = 0; j < 128; j++)
                {
                    if (!all_ents[j].active)
                        continue;
                    FEntity *target = &all_ents[j];

                    if (target->type == ENT_TYPE_ENFORCER_F || target->type == ENT_TYPE_ENFORCER_M ||
                        target->type == ENT_TYPE_UNIT4 || target->type == ENT_TYPE_BOXER || target->type == ENT_TYPE_WORKER)
                    {
                        Vec4 e_diff;
                        e_diff.x = curr_pos.x - target->pos.x;
                        e_diff.y = curr_pos.y - target->pos.y;
                        e_diff.z = curr_pos.z - target->pos.z;
                        e_diff.w = 0;

                        i32 horiz_dist = fx_sqrt(fx_mul_q16(e_diff.x, e_diff.x) + fx_mul_q16(e_diff.z, e_diff.z));
                        i32 vert_dist = e_diff.y < 0 ? -e_diff.y : e_diff.y;

                        if (horiz_dist < FX_FROM_FLOAT(0.85f) && vert_dist < FX_FROM_FLOAT(1.2f))
                        {
                            target->health -= 34; // 3 shots to kill
                            if (target->health <= 0)
                            {
                                target->active = 0;
                                if (target->type == ENT_TYPE_ENFORCER_F)
                                {
                                    audio_play_sfx_at(SFX_FEMALE_DEATH, &target->pos);
                                }
                                else
                                {
                                    audio_play_sfx_at(SFX_MALE_DEATH, &target->pos);
                                }
                                if (target->type == ENT_TYPE_BOXER)
                                    g_boxer_dead = 1;

                                // Spawn ammo drop from defeated enforcers
                                if (target->type == ENT_TYPE_ENFORCER_F || target->type == ENT_TYPE_ENFORCER_M || target->type == ENT_TYPE_UNIT4)
                                {
                                    Vec4 drop_pos = target->pos;
                                    drop_pos.y = FX_FROM_FLOAT(0.25f);
                                    fentity_spawn(ENT_TYPE_AMMO, drop_pos);
                                }
                            }
                            else
                            {
                                audio_play_sfx(SFX_HIT);
                                target->state = STATE_HIT;
                                target->timer = FX_FROM_FLOAT(0.18f); // Visible flinch
                                if (target->type == ENT_TYPE_UNIT4)
                                {
                                    g_doubt += 2;
                                }
                            }
                            hit_entity = 1;
                            break;
                        }
                    }
                }

                if (hit_entity)
                {
                    break; // Ray stopped on target impact
                }
            }
        }
        else
        {
            // Empty chamber click
            audio_play_sfx(SFX_EMPTY_GUN);
        }
    }
    prev_lmb = mouse.btn_left;
}
