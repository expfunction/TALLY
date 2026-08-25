/* Copyright (c) 2026 Burak Yazar */

#ifndef ENTITY_H
#define ENTITY_H

#include "../CORE/TYPES/WORLD.H"
#include "../RNDR/ANIM.H"
#include "../RNDR/CAMER.H"
#include "../MESH/CMS.H"
#include "../PHYS/PHYS.H"
#include "../RNDR/PRTCL.H"

#define MAX_ENTITIES 32
#define MAX_ENT_MESHES 16

/* Component flags (bitfield in u16) */
#define ENT_F_ACTIVE 0x0001
#define ENT_F_MESH 0x0002
#define ENT_F_SOUND 0x0004
#define ENT_F_LIGHT 0x0008
#define ENT_F_THINK 0x0010
#define ENT_F_ANIM 0x0020
#define ENT_F_DEAD 0x0040
/* Physics flags (defined in PHYS/PHYS.H): ENT_F_PHYS 0x0080, ENT_F_GRAVITY 0x0100 */
#define ENT_F_EMIT 0x0200
#define ENT_F_VANIM 0x0400 /* vertex animation active on this entity's mesh */

typedef enum EntType_e
{
    ENT_GENERIC,
    ENT_PROJECTILE,
    ENT_PICKUP,
    ENT_EFFECT,
    ENT_NPC,
    ENT_PLAYER
} EntType;

typedef struct Entity_s Entity;

typedef void (*EntThinkFn)(Entity *self, i32 dt);

typedef struct EntMesh_s
{
    MeshCMS *mesh;   /* shared, NOT owned; non-const to allow cva_scratch writes */
    Mat4x4 rotation; /* Q16.16 rotation matrix */
    i32 scale_fx;    /* uniform scale Q16.16 (FX_ONE = 1.0) */
} EntMesh;

typedef struct EntSound_s
{
    i8 smix_slot;   /* mixer slot index (integer, -1 = unassigned) */
    PCM8UFile *pcm; /* shared, NOT owned */
    i32 base_vol;   /* volume at min_dist Q16.16 (0..FX_ONE) */
    i32 min_dist;   /* full-volume distance Q16.16 world units */
    i32 max_dist;   /* silence distance Q16.16 world units */
    bool looping;
} EntSound;

typedef struct EntLight_s
{
    int emit_level;
} EntLight;

typedef struct EntEmitter_s
{
    int prtcl_idx; /* index into particle system emitter pool, -1 = unassigned */
} EntEmitter;

typedef struct Entity_s
{
    u16 flags; /* ENT_F_* component bitfield */
    EntType type;
    Vec4 position; /* world-space position Q16.16 */
    Vec3 velocity; /* velocity in units/sec Q16.16 */
    i32 lifetime;  /* TTL Q16.16 seconds (0 = infinite) */
    i32 age;       /* time alive Q16.16 seconds */

    EntMesh mesh;       /* if ENT_F_MESH */
    EntSound sound;     /* if ENT_F_SOUND */
    EntLight light;     /* if ENT_F_LIGHT */
    EntPhys phys;       /* if ENT_F_PHYS */
    AnimPlayer anim;    /* if ENT_F_ANIM  — keyframe transform animation */
    VAnimPlayer vanim;  /* if ENT_F_VANIM — per-vertex animation */
    EntEmitter emitter; /* if ENT_F_EMIT */
    EntThinkFn think;   /* if ENT_F_THINK */
    EntHitFn on_hit;    /* if ENT_F_PHYS — called each frame a collision is resolved */
    i32 user[4];        /* generic storage for think fn (interpretation is think-fn defined) */
} Entity;

/* Pool management */

/**
 * @brief Initialize the entity pool. Call once at startup.
 */
void ent_init(void);

/**
 * @brief Shut down the entity system, releasing all sound slots and meshes.
 */
void ent_shutdown(void);

/* Spawn / despawn */

/**
 * @brief Spawn a new entity from the pool
 * @param type entity type enum
 * @param pos initial world-space position Q16.16
 * @param flags initial component flags (ORed with ENT_F_ACTIVE)
 * @return pool index of the new entity, or -1 if pool is full
 */
int ent_spawn(EntType type, const Vec4 *pos, u16 flags);

/**
 * @brief Get a pointer to an active entity by pool index
 * @param idx pool index
 * @return pointer to the entity, or NULL if inactive or out of range
 */
Entity *ent_get(int idx);

/**
 * @brief Mark an entity for removal (cleared on next ent_update)
 * @param idx pool index
 */
void ent_kill(int idx);

/* Component setup */

/**
 * @brief Attach a mesh component to an entity
 * @param idx pool index
 * @param mesh pointer to shared mesh data (NOT owned)
 * @param scale uniform scale Q16.16
 */
void ent_set_mesh(int idx, MeshCMS *mesh, i32 scale);

/**
 * @brief Attach a spatial sound component to an entity
 * @param idx pool index
 * @param pcm pointer to shared PCM data (NOT owned)
 * @param vol base volume Q16.16
 * @param min_d minimum attenuation distance Q16.16
 * @param max_d maximum attenuation distance Q16.16
 * @param loop true for looping playback
 */
void ent_set_sound(int idx, PCM8UFile *pcm, i32 vol, i32 min_d, i32 max_d, bool loop);

/**
 * @brief Attach a point light component to an entity
 * @param idx pool index
 * @param emit_level light emission level
 */
void ent_set_light(int idx, int emit_level);

/**
 * @brief Attach a think callback to an entity
 * @param idx pool index
 * @param fn per-frame callback function
 */
void ent_set_think(int idx, EntThinkFn fn);

/**
 * @brief Attach a collision hit callback to an entity (requires ENT_F_PHYS).
 *        Called by phys_update() each frame a collision is resolved.
 *        Pass NULL to clear the callback.
 * @param idx pool index
 * @param fn callback receiving (self, EntHitInfo*)
 */
void ent_set_on_hit(int idx, EntHitFn fn);

/**
 * @brief Attach a particle emitter component to an entity
 * @param idx pool index
 * @param em emitter definition (copied into particle system)
 */
void ent_set_emitter(int idx, const ParticleEmitter *em);

/**
 * @brief Start playing an animation clip on an entity
 * @param idx pool index
 * @param clip pointer to the animation clip data
 * @param mode playback mode (ANIM_ONCE, ANIM_LOOP, ANIM_PINGPONG)
 * @param speed playback speed multiplier Q16.16
 */
void ent_play_anim(int idx, const AnimClip *clip, AnimMode mode, i32 speed);

/* Per-frame update */

/**
 * @brief Update all active entities: think, velocity, animation, sound sync, lifetime
 * @param dt delta time Q16.16
 */
void ent_update(i32 dt);

/* Rendering */

/**
 * @brief Render all entities with mesh components
 * @param cam pointer to the camera
 * @param surf pointer to the destination surface
 * @param num_lights number of lights in the lights array
 * @param lights array of point lights for mesh shading
 * @param spans per-scanline clip span table
 */
void ent_render_all(const Camera *cam, const Surface8 *surf,
                    int num_lights, const PointLight *lights,
                    const ClipSpans *spans);

/**
 * @brief Gather point lights from all entities with light components
 * @param out output array of point lights
 * @param max_out maximum number of lights to gather
 * @return number of lights written to out[]
 */
int ent_gather_lights(PointLight *out, int max_out);

/* Mesh resource cache */

/**
 * @brief Load a CMS mesh into the entity mesh cache
 * @param cms_path path to the .cms file
 * @return cache slot index, or -1 on failure
 */
int ent_load_mesh(const char *cms_path);

/**
 * @brief Get a cached mesh by slot index
 * @param slot cache slot index
 * @return pointer to the mesh, or NULL if out of range
 */
MeshCMS *ent_get_mesh(int slot);

/**
 * @brief Start vertex animation on an entity's mesh.
 *        Resolves a flat clip index across all loaded CAN files and begins playback.
 *        Requires ENT_F_MESH with a mesh that has animations loaded.
 * @param idx       pool index
 * @param flat_clip flat clip index (0 = first clip of first CAN file, etc.)
 * @param mode      playback mode
 * @param speed     playback rate Q16.16 (FX_ONE = 1x)
 */
void ent_play_vanim(int idx, int flat_clip, AnimMode mode, i32 speed);

/**
 * @brief Start a new vertex animation clip with a linear cross-fade.
 *        Freezes the current pose and lerps to the new clip over trans_dur seconds.
 *        Falls back to a hard cut when trans_dur == 0 or no animation is active.
 * @param trans_dur blend duration Q16.16 seconds
 */
void ent_play_vanim_trans(int idx, int flat_clip, AnimMode mode, i32 speed,
                          i32 trans_dur);

/**
 * @brief Free all cached entity meshes
 */
void ent_free_meshes(void);

#endif /* ENTITY_H */
