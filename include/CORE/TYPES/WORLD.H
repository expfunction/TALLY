/* Copyright (c) 2026 Burak Yazar */

#ifndef WORLD_TYPES_H
#define WORLD_TYPES_H

#include "MATH.H"
#include "AUDIO.H"
#include "MESH.H"

/* World size limits */
#define MAX_POINT_LIGHTS 256
#define MAX_WORLD_MESHES 256
#define MAX_WORLD_SOUNDS 256
#define MAX_WORLD_ENTITIES 256
#define MAX_WORLD_BILLBOARDS 256
#define MAX_PARTICLES 512
#define MAX_EMITTERS 256
#define MAX_WORLD_TRIGGERS 64

/* ---- Brushes: parametric primitives that bake into the octree triangle soup.
 * Authored in CEDIT; triangulated by brush_emit_tris() (RNDR/OCTREE.C). They
 * replace hand-authored convex Cells as the source of static world geometry. */
#define MAX_BRUSHES 256
#define BRUSH_MAX_FACES 8 /* prism: up to 6 sides + 2 caps */

typedef enum BrushType_e
{
    BR_CUBE,
    BR_QUAD,
    BR_WEDGE,
    BR_PRISM,
    BR_TYPE_COUNT
} BrushType;

typedef struct Brush_s
{
    int type;        /* BrushType */
    Vec4 origin;     /* world-space position Q16.16 */
    Vec3 rotation;   /* Euler angles Q16.16 radians (pitch, yaw, roll) */
    Vec3 size;       /* half-extents per axis Q16.16 (BR_PRISM: size.x = radius) */
    int prism_sides; /* BR_PRISM only (5 or 6) */
    i16 face_material[BRUSH_MAX_FACES];
    i16 face_heightmap[BRUSH_MAX_FACES];
    i8 face_heightmap_amp[BRUSH_MAX_FACES];
    i32 uv_scale; /* texture tiles per world unit Q16.16 (0 => default 1.0) */
} Brush;

typedef enum
{
    LCOLOR_WHITE = 0,
    LCOLOR_RED,
    LCOLOR_GREEN,
    LCOLOR_BLUE,
    LCOLOR_YELLOW,
    MAX_LIGHT_COLORS
} LightColor;

typedef struct PointLight_s
{
    Vec4 position;
    int emit_level;
    LightColor color;
} PointLight;

/* World-space quad (with optional heightmap) */
typedef struct QuadWorld_s
{
    Vec4 verts[4]; /* corner positions Q16.16 */
    Vec2 uvs[4];   /* texture coordinates Q16.16 */
    u8 *tex;
    u32 tex_w;         /* texture width in texels (integer) */
    u32 tex_h;         /* texture height in texels (integer) */
    int id;            /* editor id (integer) */
    i8 uv_scale;       /* tri-planar UV tiling shift: 2^uv_scale texels per world unit */
    u32 heightmap_w;   /* heightmap width in texels (integer) */
    u32 heightmap_h;   /* heightmap height in texels (integer) */
    i32 heightmap_amp; /* vertical displacement scale Q16.16 */
    u8 *heightmap;
    int color_key;     /* transparent palette index (-1 = opaque) */
    u8 semantic_id;    /* material/biome tag written into the neural G-buffer */
    i16 material_id;   /* index into m_mat_*[] for GPU GI material lookup */
    i32 fnx, fny, fnz; /* precomputed face normal (any scale; 0,0,0 = recompute) */
    i16 blend_texture_id;
} QuadWorld;

typedef struct WorldMesh_s
{
    MeshCMS mesh;
    Vec4 position; /* world-space position Q16.16 */
    int loaded;    /* boolean: 1 = mesh data valid */
    char name[64];
    i8 sel_clip;   /* active animation clip index (integer, -1 = none) */
    Vec3 rotation; /* Q16.16 Euler angles (pitch, yaw, roll) in radians */
} WorldMesh;

typedef struct WorldSound_s
{
    Vec4 position;          /* world-space position Q16.16 */
    i32 base_vol;           /* volume at min_dist Q16.16 (0..FX_ONE) */
    i32 min_dist, max_dist; /* attenuation range Q16.16 world units */
    bool looping;
    char name[64];
    PCM8UFile pcm;
    int loaded; /* boolean: 1 = pcm data valid */
} WorldSound;

typedef struct WorldBillboard_s
{
    Vec4 position;        /* world-space position Q16.16 */
    i32 world_w, world_h; /* billboard size in world units Q16.16 */
    i16 material_id;      /* index into m_mat_*[] */
} WorldBillboard;

typedef struct WorldEntity_s
{
    Vec4 position; /* world-space spawn position Q16.16 */
    Vec3 velocity; /* initial velocity in units/sec Q16.16 */
    u16 flags;     /* ENT_F_* component bitfield */
    u8 type;       /* EntType enum value */
    u8 think_id;   /* think function registry index (integer) */
    char mesh_name[64];
    i32 mesh_scale;    /* uniform mesh scale Q16.16 */
    i8 mesh_anim_clip; /* initial animation clip index (integer, -1 = none) */
    char snd_name[64];
    i32 snd_vol;          /* base volume Q16.16 (0..FX_ONE) */
    i32 snd_min, snd_max; /* attenuation distances Q16.16 world units */
    bool snd_loop;
    int light_emit;   /* point light emission level (integer) */
    i32 phys_radius;  /* sphere collider radius Q16.16 */
    i32 phys_height;  /* capsule collider height Q16.16 */
    u8 phys_col_mask; /* collision layer mask (COL_* bits) */
    i32 lifetime;     /* TTL Q16.16 seconds (0 = infinite) */
    Vec3 rotation;    /* Q16.16 Euler angles (pitch, yaw, roll) in radians */

    /* Runtime data */
    struct MeshCMS_s *cached_mesh; /* NULL until resolved */
} WorldEntity;

typedef struct Particle_s
{
    Vec4 pos;          /* world-space position Q16.16 */
    Vec3 vel;          /* velocity in units/sec Q16.16 */
    i32 age, lifetime; /* elapsed / total duration Q16.16 seconds */
    i32 gravity;       /* per-particle gravity override Q16.16 units/sec² */
    i32 damping;       /* velocity damping Q16.16 multiplier per second */
    u8 color;          /* palette index */
    u8 color_end;      /* palette index for end of life (interpolated index) */
    u8 active;         /* boolean: 1 = alive */
    i16 material_id;   /* billboard material index (-1 = use pixel color) */
} Particle;

typedef enum ParticlePreset_e
{
    PRTCL_PRESET_CUSTOM = 0,
    PRTCL_PRESET_FIRE,
    PRTCL_PRESET_SMOKE,
    PRTCL_PRESET_SPARK,
    PRTCL_PRESET_RAIN,
    PRTCL_PRESET_COUNT
} ParticlePreset;

typedef struct ParticleEmitter_s
{
    Vec4 position;     /* world-space origin Q16.16 */
    Vec3 vel_base;     /* base ejection velocity Q16.16 units/sec */
    Vec3 vel_rand;     /* random velocity half-range Q16.16 units/sec */
    i32 gravity;       /* downward acceleration Q16.16 units/sec² */
    i32 spawn_rate;    /* particles per second Q16.16 */
    i32 particle_life; /* particle lifetime Q16.16 seconds */
    u8 color;          /* palette index for spawned particles */
    u8 active;         /* boolean: 1 = emitting */
    i32 spawn_accum;   /* fractional particle accumulator Q16.16 */
    i32 spawn_radius;  /* spawn randomly within this radius Q16.16 */
    i32 vel_damping;   /* friction/damping Q16.16 applied to particle velocity */
    u8 color_end;      /* target end color */
    u8 preset;         /* ParticlePreset value */
    i16 material_id;   /* billboard material index (-1 = pixel particles) */
} ParticleEmitter;

typedef struct WorldTrigger_s
{
    Vec4 position;    /* world-space center Q16.16 */
    Vec3 size;        /* half-extents Q16.16 */
    int target_scene; /* target scene id */
} WorldTrigger;

typedef struct World_s
{
    int num_pointLights, num_meshes, num_sounds, num_entities, num_billboards;
    int num_emitters;
    int num_brushes;
    int num_triggers;
    Brush brushes[MAX_BRUSHES];
    PointLight pointLights[MAX_POINT_LIGHTS];
    WorldMesh meshes[MAX_WORLD_MESHES];
    WorldSound sounds[MAX_WORLD_SOUNDS];
    WorldEntity entities[MAX_WORLD_ENTITIES];
    WorldBillboard billboards[MAX_WORLD_BILLBOARDS];
    ParticleEmitter emitters[MAX_EMITTERS];
    WorldTrigger triggers[MAX_WORLD_TRIGGERS];

    i32 fog_z_start, fog_z_end; /* fog depth range Q16.16 world units */
    int sky_material_id;        /* index into m_mat_*[] for sky portal faces, = -1 if none */

} World;

#endif /* WORLD_TYPES_H */
