/* Copyright (c) 2026 Burak Yazar */

#ifndef OCTREE_H
#define OCTREE_H

/* ---- Octree triangle-soup world renderer ----
 *
 * Replaces the recursive cell/portal renderer (RNDR/CELL.C) for static world
 * geometry. World brushes (see CORE/TYPES/WORLD.H) are triangulated into one
 * global triangle soup; an octree is built over the soup for broad-phase
 * frustum culling. At runtime the tree is walked front-to-back and each leaf's
 * triangles are rasterized through the existing depth-buffered span rasterizer
 * (occlusion comes from the 1/z depth buffer, GFX/VGA.H : depth_buffer).
 *
 * The editable scene (brushes, placements, lights, ...) lives in g_world; the
 * baked soup+octree is regenerable and persisted as a ".COC" sidecar next to
 * the world's ".CWR" file.
 */

#include "../CORE/TYPES/MATH.H"
#include "../CORE/TYPES/GFX.H"
#include "../CORE/TYPES/CAM.H"
#include "../CORE/TYPES/WORLD.H"
#include "../PHYS/COLLIS.H"

/* Stop subdividing a node at/under this many triangles, or at OCT_MAX_DEPTH. */
#define OCT_MAX_DEPTH 8
#define OCT_LEAF_MAX_TRIS 64

/* Tesselation step and absolute func def. */
#define TESS_STEP FX_FROM_INT(4)
#define TESS_ABS(x) ((x) < 0 ? -(x) : (x))

/* File constants. */
#define OCTREE_MAGIC "COC5"
#define OCTREE_VERSION 5

/* Global baked triangle soup. u32 indices: a whole-world soup readily exceeds
 * the 65535-vertex limit imposed by per-mesh CMS u16 indices. */
typedef struct WorldSoup_s
{
    u32 vert_count;
    Vec4 *pos; /* world-space positions Q16.16 (w = FX_ONE) */
    Vec2 *uv;  /* normalized tile UVs Q16.16 (1.0 = one texture tile) */
    Vec2 *uv2; /* blend-map parametric UVs Q16.16 (0..1 over terrain quad, NULL if unused) */
    Vec2 *uv3; /* layer 1 UVs Q16.16 */
    Vec2 *uv4; /* layer 2 UVs Q16.16 */
    Vec2 *uv5; /* layer 3 UVs Q16.16 */
    Vec2 *uv6; /* layer 4 UVs Q16.16 */
    Vec2 *uv7; /* layer 5 UVs Q16.16 */
    Vec2 *uv8; /* layer 6 UVs Q16.16 */
    Vec2 *uv9; /* layer 7 UVs Q16.16 */

    u32 tri_count;
    u32 *idx;          /* tri_count*3 vertex indices */
    i16 *tri_material; /* tri_count global material ids (index into m_mat_*[]) */

    /* Lighting Cache */
    Vec3 *normal;          /* Per-vertex smooth normals */
    Vec3 *face_normal;     /* Per-triangle flat normals */
    i32 *baked_base_light; /* Pre-baked directional sky lighting */
    i32 *lit_level;        /* Cached diffuse light level */
    i32 *spec_level;       /* Cached specular light level */
    u8 *lit_color;         /* Cached light color index */
    u32 *lit_frame;        /* Frame number when lighting was cached */
} WorldSoup;

typedef struct OctNode_s
{
    i32 minx, miny, minz; /* loose AABB (union of contained tris) Q16.16 */
    i32 maxx, maxy, maxz;
    u32 skip_index; /* array index of the next node outside this subtree */
    u32 first_tri;  /* offset into leaf_tris (leaves only) */
    u32 tri_count;  /* 0 for internal nodes */
} OctNode;

typedef struct WorldOctree_s
{
    WorldSoup soup;
    OctNode *nodes;
    u32 node_count;
    u32 *leaf_tris; /* triangle ids grouped contiguously per leaf */
    u32 leaf_tri_count;
    int loaded; /* 1 = ready to render */
    i32 last_baked_tod;
} WorldOctree;

typedef struct FrustumPlane_s
{
    Vec3 normal;
    i32 d;
} FrustumPlane;

typedef struct Frustum_s
{
    FrustumPlane planes[6];
} Frustum;

/* Per-vertex data bundle used during near-plane clipping. */
typedef struct SoupVert_s
{
    Vec4 c;       /* camera-space position */
    Vec4 w;       /* world-space position (for lighting) */
    i32 su, sv;   /* source UV (Q16.16, pre-tex-scale) */
    i32 su2, sv2; /* blend-map parametric UV (Q16.16, 0..1 over terrain quad) */
    i32 su3, sv3;
    i32 su4, sv4;
    i32 su5, sv5;
    i32 su6, sv6;
    i32 su7, sv7;
    i32 su8, sv8;
    i32 su9, sv9;

    /* Interpolated Lighting */
    i32 light_level;
    i32 spec_level;
    u8 light_color;
} SoupVert;

/* Single global octree, owned by RNDR/OCTREE.C (pointers are heap-owned). */
extern WorldOctree g_octree;

/**
 * @brief Release all heap arrays and zero the octree.
 * @param o
 */
void octree_free(WorldOctree *o);

/**
 * @brief Triangulate one brush into the soup (grows pos/uv/idx/tri_material via realloc; vcap and icap track current capacities). Returns 1 on success.
 * @param b
 * @param s
 * @param vcap
 * @param icap
 */
int brush_emit_tris(const Brush *b, WorldSoup *s, u32 *vcap, u32 *icap);

/**
 * @brief Bake mesh triangles into triangle soup
 * @param m WorldMesh object
 * @param mesh_idx Index of mesh in world array
 * @param s WorldSoup object
 * @param vcap Vertex cap
 * @param icap Index cap
 * @return 1 on success
 */
int mesh_emit_tris(const WorldMesh *m, int mesh_idx, WorldSoup *s, u32 *vcap, u32 *icap);

/**
 * @brief Compute a brush's world-space axis-aligned bounding box (Q16.16). Folds in rotation by transforming the 8 local-extent corners. Used by physics for sphere-vs-brush collision.
 * @param b
 * @param out_min
 * @param out_max
 */
void brush_world_aabb(const Brush *b, Vec3 *out_min, Vec3 *out_max);

/**
 * @brief Build the soup from all brushes in w, then build the octree. Returns 1 on success (sets o->loaded), 0 if there is no geometry.
 * @param w
 * @param o
 */
int octree_bake_from_world(World *w, WorldOctree *o);

/**
 * @brief Persist / restore the baked soup+octree.
 * @param o
 * @param path
 */
int octree_save(const WorldOctree *o, const char *path);
int octree_load(WorldOctree *o, const char *path);
/**
 * @brief Derive "<world_path -> .COC>" and load it into o. Returns 1 if loaded.
 * @param o
 * @param world_path
 */
int octree_load_sidecar(WorldOctree *o, const char *world_path);

/**
 * @brief Render the static octree (depth write + test).
 * @param o
 * @param cam
 * @param surf
 * @param num_lights
 * @param lights
 * @param spans
 */
void render_world_octree(WorldOctree *o, const Camera *cam, Surface8 *surf,
                         int num_lights, const PointLight *lights, const ClipSpans *spans);

/**
 * @brief Test a sphere against the static world octree.
 * @param o WorldOctree object
 * @param pos Sphere center Q16.16
 * @param radius Sphere radius Q16.16
 * @param hits Output array for collision hits
 * @param max_hits Maximum number of hits to record
 * @return Number of hits found
 */
int octree_collide_sphere(WorldOctree *o, const Vec4 *pos, i32 radius, ColHit *hits, int max_hits);

/**
 * @brief Test a capsule against the static world octree.
 * @param o WorldOctree object
 * @param p0 Capsule start point Q16.16
 * @param p1 Capsule end point Q16.16
 * @param radius Capsule radius Q16.16
 * @param hits Output array for collision hits
 * @param max_hits Maximum number of hits to record
 * @return Number of hits found
 */
int octree_collide_capsule(WorldOctree *o, const Vec4 *p0, const Vec4 *p1, i32 radius, ColHit *hits, int max_hits);

#endif /* OCTREE_H */
