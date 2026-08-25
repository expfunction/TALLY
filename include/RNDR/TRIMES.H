/* Copyright (c) 2026 Burak Yazar */

/* Represents mesh triangles. Rasterization done here. */
#ifndef TRIMES_H
#define TRIMES_H

#include "../CORE/TYPES/WORLD.H"
#include "../CORE/TYPES/GFX.H"
#include "../MESH/CMS.H"
#include "CAMER.H"

/* Max vertices we can handle on the stack (Vec2 + Vec3 = 20 bytes per vert) */
#define TRIMES_MAX_STACK_VERTS 512

/**
 * @brief Rotate mesh by (ax, ay, az) in radians
 * @param m Pointer to mesh object
 * @param ax Delta X angle (Q16.16)
 * @param ay Delta Y angle (Q16.16)
 * @param az Delta Z angle (Q16.16)
 */
void trimes_rotate(MeshCMS *m, i32 ax, i32 ay, i32 az);

/**
 * @brief Scale mesh uniformly by s
 * @param m Pointer to mesh object
 * @param s Scale factor (Q16.16)
 */
void trimes_scale(MeshCMS *m, i32 s);

/**
 * @brief Transform and project mesh vertices
 * @param m Pointer to mesh object
 * @param cam Pointer to camera object
 * @param world_pos World-space position offset (or NULL for origin)
 * @param outScr Screen-space positions (out)
 * @param outTrns Camera-space positions (out Q16.16)
 * @return 1 if transofrmed and projected 0 if not.
 */
int trimes_transform_project(const MeshCMS *m, const Camera *cam,
                             const Vec4 *world_pos,
                             Vec2 *outScr,
                             Vec3 *outTrns,
                             Vec4 *outWorld);

/**
 * @brief Render whole mesh as textured triangles (per-submesh textures)
 * @param m Pointer to mesh object
 * @param cam Pointer to camera object
 * @param surf Pointer to surface object
 * @param world_pos World-space position offset (or NULL for origin)
 * @param point_light_num Number of point lights (0 = full bright)
 * @param world_lights Array of point lights
 * @param fallback_tex Fallback texture if submesh has no assigned texture (or NULL)
 * @param spans Per-scanline clip span table
 */
void draw_trimes_tex(const MeshCMS *m, const Camera *cam, const Surface8 *surf,
                     const Vec4 *world_pos,
                     int point_light_num, const PointLight *world_lights,
                     const u8 *fallback_tex, const ClipSpans *spans);

/**
 * @brief Get the world-space position of a named slot.
 * @param m Pointer to mesh object
 * @param name Name of the slot
 * @param world_pos World-space position offset (or NULL for origin)
 * @param out_world Receives the Q16.16 world position
 * @return 1 if found, 0 if not
 */
int trimes_get_slot_world_pos(const MeshCMS *m, const char *name,
                              const Vec4 *world_pos, Vec4 *out_world);

#endif
