/* Copyright (c) 2026 Burak Yazar */

#ifndef MMATERL_H
#define MMATERL_H

/* ---- Material Manager ----
 *
 * Materials describe how a surface looks and contributes to lighting.
 * Each surface (CellFace, CMS submesh, billboard, sky portal) carries an
 * i16 material_id that indexes into the Structure-of-Arrays buffers below.
 *
 * Storage layout: SoA (Structure-of-Arrays). Individual channels
 * (e.g. albedo RGB, emissive RGB) are accessed without pulling unrelated
 * fields into cache. Init mirrors the texture manager: auto-seed one material per
 * loaded texture, then apply per-material .CML text overrides found in
 * ASSTS/MATL/.
 */

#include "TYPES/GFX.H"
#include "MTEXTUR.H"

#define MAX_MATERIALS 256
/* face_world_uvs shift: 2^6=64 texels per world unit (matches CEDIT.C). */
#define MATERIALS_DEFAULT_UV_SCALE 6

/* flags */
#define MAT_FLAG_NONE 0x00
#define MAT_FLAG_TRANSPARENT 0x01 /* uses color_key */
#define MAT_FLAG_SKY 0x02
#define MAT_FLAG_ANIMATED 0x04
#define MAT_FLAG_EMISSIVE 0x08 /* emissive_intensity > 0 */
#define MAT_FLAG_TERRAIN 0x10
#define MAT_FLAG_GRASS 0x20
#define MAT_FLAG_EXPLICIT 0x40 /* loaded from .CML override */

/* Identity / ownership */
extern u8 *m_mat_name[MAX_MATERIALS]; /* heap-strdup'd, NULL = empty slot */

/* Texture references */
extern i16 m_mat_texture_id[MAX_MATERIALS];     /* into m_textures[]; -1 untextured */
extern i16 m_mat_lod_texture_id[MAX_MATERIALS]; /* LOD texture swap */
extern i32 m_mat_lod_distance[MAX_MATERIALS];   /* LOD swap distance threshold (Q16) */
extern i16 m_mat_heightmap_id[MAX_MATERIALS];   /* into m_height_textures[]; -1 none */
extern i8 m_mat_heightmap_amp[MAX_MATERIALS];

/* Surface props (formerly per-face on CellFace) */
extern i16 m_mat_color_key[MAX_MATERIALS]; /* transparent palette index, -1 opaque */
extern u8 m_mat_alpha[MAX_MATERIALS];      /* 0 = invisible, 255 = fully opaque */
extern u8 m_mat_semantic_id[MAX_MATERIALS];
extern i8 m_mat_uv_scale[MAX_MATERIALS]; /* face_world_uvs shift, default 6 */

/* Lighting / material properties */
extern u8 m_mat_albedo_idx[MAX_MATERIALS]; /* palette index */
extern u8 m_mat_albedo_r[MAX_MATERIALS];
extern u8 m_mat_albedo_g[MAX_MATERIALS];
extern u8 m_mat_albedo_b[MAX_MATERIALS];
extern u8 m_mat_emissive_r[MAX_MATERIALS];
extern u8 m_mat_emissive_g[MAX_MATERIALS];
extern u8 m_mat_emissive_b[MAX_MATERIALS];
extern u8 m_mat_emissive_intensity[MAX_MATERIALS]; /* 0 non-emissive */
extern u8 m_mat_roughness[MAX_MATERIALS];          /* 0 mirror, 255 fully diffuse */
extern u8 m_mat_metalness[MAX_MATERIALS];          /* 0 dielectric, 255 full metal */
extern u8 m_mat_flags[MAX_MATERIALS];

/* PBR texture maps (-1 = not set; populated on non-DOS builds only). */
extern i16 m_mat_normalmap_id[MAX_MATERIALS]; /* into m_nrml_textures[]  */
extern i16 m_mat_roughmap_id[MAX_MATERIALS];  /* into m_roug_textures[]  */
extern i16 m_mat_metalmap_id[MAX_MATERIALS];  /* into m_metal_textures[] */
extern i16 m_mat_specmap_id[MAX_MATERIALS];   /* into m_spec_textures[]  */

/* ---- Lifecycle ---- */

/**
 * @brief Auto-seed one material per populated m_textures[i], then overlay any .CML files found in ASSTS\MATL\.  Call after textures_init(). Returns 1 on success.
 */
int materials_init(void);

void materials_free(void);

/**
 * @brief Returns slot index or -1. Case-insensitive name match.
 * @param name
 */
int materials_find_by_name(const char *name);

/**
 * @brief Parse a single .CML text file into the given slot. Used by materials_init and exposed for tooling. Returns 1 on success.
 * @param path
 * @param slot
 */
int materials_load_cml(const char *path, int slot);

/**
 * @brief Returns the first empty material slot, or -1 if all 256 are occupied.
 */
int materials_find_empty_slot(void);

/**
 * @brief Backward-compatibility helper for legacy CWR files: returns a material id whose (texture, heightmap, color_key, semantic, uv_scale, hm_amp) tuple matches the legacy face properties.  Dedupes against existing slots; if the tuple is new, allocates a derived slot named "<texname>#<n>". Returns 0 (always-valid fallback) if no slot can be allocated.
 * @param tex_idx
 * @param uv_scale
 * @param heightmap_id
 * @param heightmap_amp
 * @param color_key
 * @param semantic_id
 */
int materials_find_or_create_legacy(i16 tex_idx, i8 uv_scale,
                                    i8 heightmap_id, i8 heightmap_amp,
                                    i16 color_key, u8 semantic_id);

#define MAT_ID_TERRAIN_BLEND_BASE 10000
#define MAX_TERRAIN_BLENDS 1024

typedef struct TerrainBlendProfile_s
{
    i16 mat8[8];
    const u8 *blend_map;
    u32 blend_w;
    u32 blend_h;
    i16 blend_texture_id;
} TerrainBlendProfile;

extern TerrainBlendProfile m_terrain_blends[MAX_TERRAIN_BLENDS];

int materials_register_terrain_blend(const i16 *mat8, const u8 *blend_map, u32 blend_w, u32 blend_h, i16 bmid);
void materials_clear_terrain_blends(void);
int materials_get_num_terrain_blends(void);
TerrainBlendProfile *materials_get_terrain_blend(int index);

#endif /* MMATERL_H */
