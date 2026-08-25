/* Copyright (c) 2026 Burak Yazar */

#ifndef TRIGR_H
#define TRIGR_H

#include "CORE/CENG.H"

typedef struct
{
    Vec4 min;
    Vec4 max;
} TriggerAABB;

typedef struct
{
    int active;
    int scene_id_target;
    TriggerAABB bounds;
} TriggerVolume;

/* Maximum number of triggers in the level */
#define MAX_TRIGGERS 32

/**
 * @brief Initializes the trigger system
 */
void trigr_init(void);

/**
 * @brief Adds a new trigger volume
 * @param min Bottom-left-near corner
 * @param max Top-right-far corner
 * @param target_scene The scene ID this trigger transitions to
 * @return 1 if successfully added, 0 if full
 */
int trigr_add_volume(const Vec4 *min, const Vec4 *max, int target_scene);

/**
 * @brief Checks if a given point is inside any active trigger volume.
 * @param pos The point to check (e.g., player's position)
 * @return target_scene ID if inside a volume, -1 otherwise.
 */
int trigr_check_point(const Vec4 *pos);

#endif /* TRIGR_H */
