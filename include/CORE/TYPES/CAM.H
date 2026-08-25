/* Copyright (c) 2026 Burak Yazar */

#ifndef CAM_TYPES_H
#define CAM_TYPES_H

#include "MATH.H"

typedef enum CamProjMode_e
{
    CAM_PROJ_PERSPECTIVE
} CamProjMode;

typedef struct Camera_s
{
    Vec4 position; /* world-space position Q16.16 */

    Vec3 up, right, forward; /* camera basis vectors Q16.16 */

    i32 pitch, yaw, roll; /* orientation Q16.16 radians */

    i32 z_near, z_far;            /* depth clip planes Q16.16 */
    i32 f;                        /* focal length Q16.16 */
    i32 principal_x, principal_y; /* optical centre in screen pixels (integer) */

    /* Pre-calculated Q16.16 frustum slopes */
    i32 fov_slope_x;
    i32 fov_slope_y;

    Mat4x4 view, projection, viewproj; /* Q16.16 matrices */

    CamProjMode project_mode;
} Camera;

#endif /* CAM_TYPES_H */
