/* Copyright (c) 2026 Burak Yazar */

#ifndef CAMER_H
#define CAMER_H

#include "../CORE/TYPES/CAM.H"
#include "../CORE/FIXED.H"

/**
 * @brief Initializes camera with default properties.
 * @param cam Pointer to camera object.
 * @param mode Projection mode.
 */
void cam_init(Camera *cam, const Vec3 *pos, const Vec3 *rot, const CamProjMode mode);

/**
 * @brief Update camera matrices from position/orientation
 * @param cam Camera pointer
 * @param mode projection mode
 */
void cam_update(Camera *cam, const CamProjMode mode);

/**
 * @brief Move camera along its local axes
 * @param cam Pointer to camera object
 * @param dx move along local +X (in Q16.16)
 * @param dy move along local +Y (in Q16.16)
 * @param dz move along local +Z (in Q16.16)
 */
void cam_move_local(Camera *cam, i32 dx, i32 dy, i32 dz);

/**
 * @brief Rotate camera around its local axes
 * @param cam Pointer to camera object
 * @param dyaw delta yaw (in radians, Q16.16)
 * @param dpitch delta pitch (in radians, Q16.16)
 * @param droll delta roll (in radians, Q16.16)
 */
void cam_rotate_local(Camera *cam, i32 dyaw, i32 dpitch, i32 droll);

/**
 * @brief Project using pinhole camera model:
 * @note sx = f * x / z + cx
 * @note sy = f * y / z + cy
 * @note where f is focal length in pixels, (cx, cy) is principal point (screen center)
 * @param cam Pointer to camera object
 * @param v Input vector in camera space (Q16.16)
 * @param out Output vector in screen space (Q16.16)
 */
void cam_project_pers(const Camera *cam, const Vec4 *v, Vec2 *out);

/**
 * @brief Transform point from world space to camera space
 * @param cam Pointer to camera object
 * @param world input point in world space (Q16.16)
 * @param out output point in camera space (Q16.16)
 */
void cam_world_to_cam(const Camera *cam, const Vec4 *world, Vec4 *out);

/**
 * @brief Position and orient camera to follow a target (third-person).
 *        Places the camera arm_len units behind and arm_height units above
 *        the target, looking at the target with the given pitch.
 *        Call cam_update() after this to rebuild matrices.
 * @param cam        Camera to position
 * @param target     World-space position to follow (Q16.16)
 * @param yaw        Horizontal facing angle of the target (Q16.16 radians)
 * @param arm_len    Distance behind target along facing (Q16.16)
 * @param arm_height Height above target (Q16.16)
 * @param pitch      Camera pitch downward (negative = looking down, Q16.16 radians)
 * @param lag
 */
void cam_follow(Camera *cam, const Vec4 *target, i32 yaw,
                i32 arm_len, i32 arm_height, i32 pitch, i32 lag, i32 xOffset);

#endif
