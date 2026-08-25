/* Copyright (c) 2026 Burak Yazar */

#ifndef COLLIS_H
#define COLLIS_H

#include "../CORE/TYPES/WORLD.H"
#include "../CORE/FIXED.H"

/* Collision hit result */
typedef struct ColHit_s
{
   Vec3 normal; /* push-out direction (normalized) */
   i32 penetr;  /* penetration depth Q16.16 (positive = overlapping) */
} ColHit;

#define MAX_COL_HITS 4

/* Collision primitives */

/**
 * @brief Test sphere vs sphere overlap
 * @param pos_a center of sphere A (world-space Q16.16)
 * @param rad_a radius of sphere A Q16.16
 * @param pos_b center of sphere B (world-space Q16.16)
 * @param rad_b radius of sphere B Q16.16
 * @param hit output collision hit (normal points from B to A)
 * @return 1 if overlapping, 0 otherwise
 */
int col_sphere_sphere(const Vec4 *pos_a, i32 rad_a,
                      const Vec4 *pos_b, i32 rad_b,
                      ColHit *hit);

/**
 * @brief Test a sphere against a solid axis-aligned box (world-space).
 *        Brushes collide as their world AABB; the sphere is pushed out along
 *        the minimum-penetration axis. Normal points away from the box.
 * @param pos sphere center (world-space Q16.16)
 * @param radius sphere radius Q16.16
 * @param bmin box minimum corner Q16.16
 * @param bmax box maximum corner Q16.16
 * @param hit output collision hit
 * @return 1 if overlapping, 0 otherwise
 */
int col_sphere_aabb(const Vec4 *pos, i32 radius,
                    const Vec3 *bmin, const Vec3 *bmax, ColHit *hit);

/**
 * @brief Test a sphere against a triangle.
 * @param pos sphere center (world-space Q16.16)
 * @param radius sphere radius Q16.16
 * @param v0, v1, v2 triangle vertices (world-space Q16.16)
 * @param hit output collision hit
 * @return 1 if overlapping, 0 otherwise
 */
int col_sphere_triangle(const Vec4 *pos, i32 radius,
                        const Vec4 *v0, const Vec4 *v1, const Vec4 *v2,
                        ColHit *hit);

/**
 * @brief Test a capsule against a triangle.
 * @param p0 capsule start point (bottom) (world-space Q16.16)
 * @param p1 capsule end point (top) (world-space Q16.16)
 * @param radius capsule radius Q16.16
 * @param v0, v1, v2 triangle vertices (world-space Q16.16)
 * @param hit output collision hit
 * @return 1 if overlapping, 0 otherwise
 */
int col_capsule_triangle(const Vec4 *p0, const Vec4 *p1, i32 radius,
                         const Vec4 *v0, const Vec4 *v1, const Vec4 *v2,
                         ColHit *hit);

/**
 * @brief Test capsule vs capsule overlap.
 * @param a0, a1 capsule A segment endpoints Q16.16
 * @param rad_a capsule A radius Q16.16
 * @param b0, b1 capsule B segment endpoints Q16.16
 * @param rad_b capsule B radius Q16.16
 * @param hit output collision hit (normal points from B to A)
 * @return 1 if overlapping, 0 otherwise
 */
int col_capsule_capsule(const Vec4 *a0, const Vec4 *a1, i32 rad_a,
                        const Vec4 *b0, const Vec4 *b1, i32 rad_b,
                        ColHit *hit);

#endif /* COLLIS_H */
