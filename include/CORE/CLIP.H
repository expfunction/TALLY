/* Copyright (c) 2026 Burak Yazar */

#ifndef CLIP_H
#define CLIP_H

#include "TYPES.H"

/* Maximum vertices poly_clip_plane can produce (input + one new per edge). */
#define POLY_CLIP_MAX 16

/**
 * @brief Sutherland-Hodgman half-space clip in-place. Clips verts against nx*x + ny*y + nz*z >= d (all Q16.16). world_verts, uvs, and lights may be NULL to skip attribute interpolation. Returns new vertex count; 0 = fully clipped.
 * @param verts
 * @param world_verts
 * @param uvs
 * @param lights
 * @param n
 * @param nx
 * @param ny
 * @param nz
 * @param d
 */
int poly_clip_plane(Vec4 *verts, Vec4 *world_verts, Vec2 *uvs, i32 *lights, int n,
                    i32 nx, i32 ny, i32 nz, i32 d);

/**
 * @brief Clip against all five camera frustum planes (near, left, right, top, bottom). Calls poly_clip_plane per plane; stops early if fully clipped. world_verts, uvs, and lights may be NULL. Returns final vertex count; 0 = fully clipped.
 * @param verts
 * @param world_verts
 * @param uvs
 * @param lights
 * @param n
 * @param cam
 */
int poly_clip_frustum(Vec4 *verts, Vec4 *world_verts, Vec2 *uvs, i32 *lights, int n,
                      const Camera *cam);

#endif /* CLIP_H */
