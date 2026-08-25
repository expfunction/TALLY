/* Copyright (c) 2026 Burak Yazar */

#ifndef MTEXTUR_H
#define MTEXTUR_H

/* ---- Texture Manager ---- */
#include "TYPES/GFX.H"

/**
 * @brief Texture dimensions
 */
#define MAX_TEXTURE_WIDTH 128  /* Width of a texture in pixels */
#define MAX_TEXTURE_HEIGHT 128 /* Height of a texture in pixels */

/**
 * @brief Maximum number of textures
 */
#define MAX_TEXTURES 256

/**
 * @brief Array of textures managed by the texture manager - these are used as brush face textures
 */
extern Sprite m_textures[MAX_TEXTURES];

/**
 * @brief Array of textures managed by the texture manager - these are used as sky textures
 */
extern Sprite m_sky_textures[MAX_TEXTURES];

/**
 * @brief Array of textures managed by the texture manager - these are used as heightmaps for brush faces
 */
extern Sprite m_terrain_textures[MAX_TEXTURES];

/**
 * @brief Array of textures managed by the texture manager - these are used as blendmaps for brush faces
 */
extern Sprite m_terrain_blend_textures[MAX_TEXTURES];

/**
 * @brief Array of textures managed by the texture manager - these are used as heightmaps for PBR
 */
extern Sprite m_height_textures[MAX_TEXTURES];

/* ---- PBR channel textures (greyscale, 1 byte/texel) ----
 * Loaded only on non-DOS (PLATFORM_WIN64) builds.
 * Roughness, metalness, and specular maps share the Sprite format.  */
extern Sprite m_roug_textures[MAX_TEXTURES];  /* roughness  ASSTS/TEXTR/ROUG/ */
extern Sprite m_metal_textures[MAX_TEXTURES]; /* metalness  ASSTS/TEXTR/METAL/ */
extern Sprite m_spec_textures[MAX_TEXTURES];  /* specular   ASSTS/TEXTR/SPEC/ */

/* ---- Normal map textures (RGB, 3 bytes/texel) ----
 * Loaded only on non-DOS (PLATFORM_WIN64) builds.
 * rgb layout: r0g0b0 r1g1b1 ... row-major.  */
typedef struct NrmSprite_s
{
    u8 *rgb; /* NULL = not loaded */
    u16 w, h;
    u8 *name; /* heap-strdup'd basename */
} NrmSprite;
extern NrmSprite m_nrml_textures[MAX_TEXTURES]; /* normal maps ASSTS/TEXTR/NRML/ */

/**
 * @brief Returns slot index in arr or -1.  name = basename (e.g. "BARK.RAW"). Case-insensitive.  Pass any Sprite array (m_textures, m_roug_textures …).
 * @param arr
 * @param n
 * @param name
 */
int textures_find_by_name(const Sprite *arr, int n, const char *name);
int textures_nrml_find_by_name(const char *name);

/**
 * @brief Initialize the texture manager
 * @return 0 on success, non-zero on failure
 */
int textures_init();

/**
 * @brief Free all textures managed by the texture manager
 */
void textures_free();

/**
 * @brief Defragment the texture memory
 * @return 0 on success, non-zero on failure
 */
int textures_defrag();

/**
 * @brief Load a palette from a file and apply it to the textures
 * @param path path to the palette file
 * @return 0 on success, non-zero on failure
 */
int load_palette_and_apply(const char *path);

/**
 * @brief Load a raw 8-bit texture from a file
 * @param path path to the raw texture file
 * @param dst pointer to the destination buffer
 * @param w width of the texture
 * @param h height of the texture
 * @return 0 on success, non-zero on failure
 */
int load_raw8(const char *path, u8 *dst, u32 w, u32 h);

#endif /* MTEXTUR_H */