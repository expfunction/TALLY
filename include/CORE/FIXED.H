/* Copyright (c) 2026 Burak Yazar */

#ifndef FIXED_H
#define FIXED_H

#include "TYPES/MATH.H"

/* ---- Q16.16 basics ---- */

/**
 * @brief Constants for Q16.16 fixed-point arithmetic
 */

#define FX_TAB_MAX_DEN 65535 /* max denominator for fx_div_q16_safe table */

/**
 * @brief Conversion macros for Q16.16 fixed-point arithmetic
 */

#define FX_FROM_INT(x) ((i32)((x) << 16))
#define FX_TO_INT(x) ((i32)((x) >> 16))
#define FX_FROM_FLOAT(x) ((i32)((x) * 65536.0f))
#define FX_FROM_FLOAT_R(x) ((i32)((x) * 65536.0f + 0.5f))
#define FX_TO_FLOAT(x) ((float)(x) / 65536.0f)
#define FX_FROM_DOUBLE(x) ((i32)((x) * 65536.0))
#define FX_TO_DOUBLE(x) ((double)(x) / 65536.0)

/**
 * @brief Common Q16.16 constants
 */

#define FX_ONE FX_FROM_INT(1)                              /* 1.0 in Q16.16 */
#define FX_TWO FX_FROM_INT(2)                              /* 2.0 in Q16.16 */
#define FX_HALF FX_FROM_FLOAT(0.5f)                        /* 0.5 in Q16.16 */
#define FX_PI (i32)(3.1415927f * 65536.0f)                 /* (pi * 65536) */
#define FX_HALF_PI (i32)(1.5707964f * 65536.0f)            /* (pi/2 * 65536) */
#define FX_TWO_PI (i32)(6.2831854f * 65536.0f)             /* (2*pi * 65536) */
#define FX_DEG2RAD (i32)(0.017453292f * 65536.0f)          /* (pi/180 * 65536) */
#define FX_RAD2DEG (i32)(57.29578f * 65536.0f)             /* (180/pi * 65536) */
#define FX_CORDIC_K (i32)(0.60725296f * 65536.0f)          /* (0.607252935 * 65536) */
#define FX_PI_R (i32)(3.1415927f * 65536.0f + 0.5f)        /* round(pi * 65536) */
#define FX_HALF_PI_R (i32)(1.5707964f * 65536.0f + 0.5f)   /* round(pi/2 * 65536) */
#define FX_TWO_PI_R (i32)(6.2831854f * 65536.0f + 0.5f)    /* round(2*pi * 65536) */
#define FX_DEG2RAD_R (i32)(0.017453292f * 65536.0f + 0.5f) /* round(pi/180 * 65536) */
#define FX_RAD2DEG_R (i32)(57.29578f * 65536.0f + 0.5f)    /* round(180/pi * 65536) */
#define FX_CORDIC_K_R (i32)(0.60725296f * 65536.0f + 0.5f) /* round(0.607252935 * 65536) */

#define LUT_SIZE 65536
#define RAD_TO_IDX (i64)(10430.378196 * 65536.0)

/**
 * @brief ---- Maintenance Functs ----
 */
extern void fx_print_consts(void);

/* ---- 64-bit-accurate Q16.16 operations ---- */

/**
 * @brief Lookup table for fx_div_q16_safe
 */
extern i32 recip_table_div[];

/**
 * @brief Multiply two Q16.16 fixed-point numbers using assembly proc
 * @param a first operand
 * @param b second operand
 * @return (a * b) >> 16
 */
extern i32 fx_mul_q16(i32 a, i32 b);

/**
 * @brief Divide two Q16.16 fixed-point numbers
 * @param num numerator
 * @param den denominator
 * @return (num << 16) / den
 */
extern i32 fx_div_q16(i32 num, i32 den);

/**
 * @brief Safe division for Q16.16 fixed-point numbers
 * @param a numerator
 * @param b denominator
 * @return (a << 16) / b, or 0 if b == 0
 */
extern i32 fx_div_q16_safe(i32 a, i32 b);

/**
 * @brief Saturated division for Q16.16 fixed-point numbers
 * @param num_q16 numerator
 * @param den_q16 denominator
 * @return saturated result of division
 */
extern i32 fx_div_q16_sat(i32 num_q16, i32 den_q16);

/**
 * @brief Table-assisted saturated division for Q16.16 fixed-point numbers
 * @param num_q16 numerator
 * @param den_q16 denominator
 * @return saturated result of division using lookup table
 */
extern i32 fx_div_q16_sat_tab(i32 num_q16, i32 den_q16);

/* ---- Trigonometric functions ---- */

/**
 * @brief Initialize lookup tables using CORDIC algorithm
 */
void fixed_init_tables(void);

/**
 * @brief Compute the ceiling of a Q16.16 fixed-point number
 * @param x input value
 * @return ceiling of x in standard integer format
 */
int fx_ceil(i32 x);

/**
 * @brief Compute the floor of a Q16.16 fixed-point number
 * @param x input value
 * @return floor of x in standard integer format
 */
int fx_floor(i32 x);

/**
 * @brief Compute sine of an angle in Q16.16 radians
 * @param angle16_16 angle in Q16.16 radians
 * @return sine of the angle in Q16.16
 */
i32 fx_sin(i32 angle16_16);

/**
 * @brief Compute cosine of an angle in Q16.16 radians
 * @param angle16_16 angle in Q16.16 radians
 * @return cosine of the angle in Q16.16
 */
i32 fx_cos(i32 angle16_16);

/**
 * @brief Convert degrees to radians in Q16.16
 * @param deg_q16 angle in degrees (Q16.16)
 * @return angle in radians (Q16.16)
 */
i32 fx_deg_to_rad(i32 deg_q16);

/**
 * @brief Compute tangent of an angle in Q16.16 radians
 * @param angle_rad_q16 angle in Q16.16 radians
 * @return tangent of the angle in Q16.16
 */
i32 fx_tan(i32 angle_rad_q16);

/* ---- Miscellaneous helpers ---- */

/**
 * @brief Convert an 8-bit value to a 6-bit DAC value
 * @param v8 8-bit value
 * @return 6-bit DAC value
 */
u8 to_dac6(int v8);

/**
 * @brief Convert a hexadecimal character to its nibble value
 * @param c hexadecimal character
 * @return nibble value
 */
int hex2nib(int c);

/**
 * @brief Compute square root of a Q16.16 fixed-point number using Newton-Raphson
 * @param x input value
 * @return square root of x in Q16.16
 */
i32 fx_sqrt(i32 x);

/**
 * @brief Swap two integers
 * @param a pointer to the first integer
 * @param b pointer to the second integer
 */
void fx_swap(i32 *a, i32 *b);

/* ---- Basic operations ---- */

/**
 * @brief Compute the minimum of two integers
 * @param a first integer
 * @param b second integer
 * @return minimum of a and b
 */
int i_min(int a, int b);

/**
 * @brief Compute the maximum of two integers
 * @param a first integer
 * @param b second integer
 * @return maximum of a and b
 */
int i_max(int a, int b);

/**
 * @brief Clamp a value between a minimum and maximum
 * @param v value to clamp
 * @param vmin minimum value
 * @param vmax maximum value
 * @return clamped value
 */
int i_clamp(int v, int vmin, int vmax);

/**
 * @brief Compute the absolute value of an integer
 * @param v input value
 * @return absolute value of v
 */
int i_abs(int v);

/**
 * @brief Perform linear interpolation between two integers
 * @param a start value
 * @param b end value
 * @param t interpolation factor (0..65536)
 * @return interpolated value
 */
int i_lerp(int a, int b, int t);

/**
 * @brief Perform clamped linear interpolation between two integers
 * @param a start value
 * @param b end value
 * @param t interpolation factor (0..65536)
 * @return interpolated value, clamped to [a, b]
 */
int i_lerp_clamped(int a, int b, int t);

/**
 * @brief Compute modulus of an integer
 * @param x dividend
 * @param m divisor
 * @return x mod m, result in [0, m)
 */
int i_mod(int x, int m);

/* ---- Q16.16 operations ---- */

/**
 * @brief Compute the absolute value of a Q16.16 fixed-point number
 * @param v input value
 * @return absolute value of v
 */
i32 fx_abs(i32 v);

/**
 * @brief Compute the minimum of two Q16.16 fixed-point numbers
 * @param a first value
 * @param b second value
 * @return minimum of a and b
 */
i32 fx_min(i32 a, i32 b);

/**
 * @brief Compute the maximum of two Q16.16 fixed-point numbers
 * @param a first value
 * @param b second value
 * @return maximum of a and b
 */
i32 fx_max(i32 a, i32 b);

/**
 * @brief Clamp a Q16.16 fixed-point value between a minimum and maximum
 * @param v value to clamp
 * @param vmin minimum value
 * @param vmax maximum value
 * @return clamped value
 */
i32 fx_clamp(i32 v, i32 vmin, i32 vmax);

/**
 * @brief Perform linear interpolation between two Q16.16 fixed-point numbers
 * @param a start value
 * @param b end value
 * @param t interpolation factor (0..65536)
 * @return interpolated value
 */
i32 fx_lerp(i32 a, i32 b, i32 t);

/**
 * @brief Perform clamped linear interpolation between two Q16.16 fixed-point numbers
 * @param a start value
 * @param b end value
 * @param t interpolation factor (0..65536)
 * @return interpolated value, clamped to [a, b]
 */
i32 fx_lerp_clamped(i32 a, i32 b, i32 t);

/**
 * @brief Compute modulus of a Q16.16 fixed-point number
 * @param x dividend
 * @param m divisor
 * @return x mod m, result in [0, m)
 */
i32 fx_mod(i32 x, i32 m);
/* ---- Matrix Operations ---- */

/**
 * @brief Prints out given matrix to console
 * @param m Pointer to matrix.
 */
void mat4x4_print(const Mat4x4 *m);

/**
 * @brief Set a 4x4 matrix to the identity matrix.
 * @param m Pointer to the matrix to set.
 */
void mat4x4_identity(Mat4x4 *m);

/**
 * @brief Multiply two 4x4 matrices.
 * @param a Pointer to the first matrix.
 * @param b Pointer to the second matrix.
 * @param out Pointer to the output matrix where the result will be stored.
 */
void mat4x4_mul(const Mat4x4 *a, const Mat4x4 *b, Mat4x4 *out);

/**
 * @brief Scales 4x4 matrix.
 * @param sx Scale on X axis.
 * @param sy Scale on Y axis.
 * @param sz Scale on Z axis.
 * @param m Pointer to output matrix.
 */
void mat4x4_scale(i32 sx, i32 sy, i32 sz, Mat4x4 *m);

/**
 * @brief Multiply a 4x4 matrix by a 4D vector.
 * @param m Pointer to the matrix.
 * @param in Pointer to the input vector.
 * @param out Pointer to the output vector where the result will be stored.
 */
void mat4x4_mul_vec4(const Mat4x4 *m, const Vec4 *in, Vec4 *out);

/**
 * @brief Sets up the given matrix for perspective projection operations.
 * @param fov_rad Field of view angle in radians.
 * @param aspect Aspect ratio of screen in radians.
 * @param znear Near plane distance in Q16.16
 * @param zfar Far plane distance in Q16.16
 * @param out Pointer to output matrix.
 */
void mat4x4_perspective(i32 fov_rad, i32 aspect, i32 znear, i32 zfar, Mat4x4 *out);

/**
 * @brief Computes the LookAt matrix
 * @param eye Position vector of view point.
 * @param target Position vector of target point.
 * @param up Camera's up vector.
 * @param out Pointer to output matrix.
 */
void mat4x4_lookat(const Vec3 *eye, const Vec3 *target, const Vec3 *up, Mat4x4 *out);

/**
 * @brief Create a 4x4 rotation matrix from Euler angles.
 * @param ax Rotation angle around the x-axis in Q16.16 radians fixed-point format.
 * @param ay Rotation angle around the y-axis in Q16.16 radians fixed-point format.
 * @param az Rotation angle around the z-axis in Q16.16 radians fixed-point format.
 * @param m Pointer to the output matrix where the result will be stored.
 */
void mat4x4_from_euler(i32 ax, i32 ay, i32 az, Mat4x4 *m);

/**
 * @brief Create a 4x4 translation matrix.
 * @param tx Translation along the x-axis in Q16.16 fixed-point format.
 * @param ty Translation along the y-axis in Q16.16 fixed-point format.
 * @param tz Translation along the z-axis in Q16.16 fixed-point format.
 * @param m Pointer to the output matrix where the result will be stored.
 */
void mat4x4_translate(i32 tx, i32 ty, i32 tz, Mat4x4 *m);

/* ---- Vector operations ---- */

/**
 * @brief Print a 2D vector
 * @param v pointer to the vector
 */
void vec2_print(const Vec2 *v);

/**
 * @brief Add two 2D vectors
 * @param a first vector
 * @param b second vector
 * @param out output vector
 */
void vec2_add(const Vec2 *a, const Vec2 *b, Vec2 *out);

/**
 * @brief Subtract two 2D vectors
 * @param a first vector
 * @param b second vector
 * @param out output vector
 */
void vec2_sub(const Vec2 *a, const Vec2 *b, Vec2 *out);

/**
 * @brief Scale a 2D vector by a scalar
 * @param v input vector
 * @param s scalar value
 * @param out output vector
 */
void vec2_scale(const Vec2 *v, i32 s, Vec2 *out);

/**
 * @brief Compute the dot product of two 2D vectors
 * @param a first vector
 * @param b second vector
 * @return dot product
 */
i32 vec2_dot(const Vec2 *a, const Vec2 *b);

/**
 * @brief Compute the cross product of two 2D vectors using the 2D cross product formula (u.x * v.y - u.y * v.x)
 * @param a first vector
 * @param b second vector
 * @return Scalar area between input vectors
 */
i32 vec2_cross(const Vec2 *a, const Vec2 *b);

/**
 * @brief Compute the squared length of a 2D vector
 * @param a input vector
 * @return squared length in Q16.16
 */
i32 vec2_length_sq(const Vec2 *a);

/**
 * @brief Compute the length of a 2D vector
 * @param a input vector
 * @return length in Q16.16
 */
i32 vec2_length(const Vec2 *a);

/**
 * @brief Compute the squared distance between two 2D vectors
 * @param a first vector
 * @param b second vector
 * @return squared distance in Q16.16
 */
i32 vec2_dist_sq(const Vec2 *a, const Vec2 *b);

/**
 * @brief Compute the distance between two 2D vectors
 * @param a first vector
 * @param b second vector
 * @return distance in Q16.16
 */
i32 vec2_dist(const Vec2 *a, const Vec2 *b);

/**
 * @brief Normalize a 2D vector
 * @param v input vector
 * @param out output vector
 */
void vec2_normalize(const Vec2 *v, Vec2 *out);

/**
 * @brief Print a 3D vector
 * @param v pointer to the vector
 */
void vec3_print(const Vec3 *v);

/**
 * @brief Add two 3D vectors
 * @param a first vector
 * @param b second vector
 * @param out output vector
 */
void vec3_add(const Vec3 *a, const Vec3 *b, Vec3 *out);

/**
 * @brief Subtract two 3D vectors
 * @param a first vector
 * @param b second vector
 * @param out output vector
 */
void vec3_sub(const Vec3 *a, const Vec3 *b, Vec3 *out);

/**
 * @brief Scale a 3D vector by a scalar
 * @param v input vector
 * @param s scalar value
 * @param out output vector
 */
void vec3_scale(const Vec3 *v, i32 s, Vec3 *out);

/**
 * @brief Compute the dot product of two 3D vectors
 * @param a first vector
 * @param b second vector
 * @return dot product
 */
i32 vec3_dot(const Vec3 *a, const Vec3 *b);

/**
 * @brief Compute the cross product of two 3D vectors
 * @param a first vector
 * @param b second vector
 * @param out output vector
 */
void vec3_cross(const Vec3 *a, const Vec3 *b, Vec3 *out);

/**
 * @brief Compute the squared length of a 3D vector
 * @param a input vector
 * @return squared length in Q16.16
 */
i32 vec3_length_sq(const Vec3 *a);

/**
 * @brief Compute the length of a 3D vector
 * @param a input vector
 * @return length in Q16.16
 */
i32 vec3_length(const Vec3 *a);

/**
 * @brief Compute the squared distance between two 3D vectors
 * @param a first vector
 * @param b second vector
 * @return squared distance in Q16.16
 */
i32 vec3_dist_sq(const Vec3 *a, const Vec3 *b);

/**
 * @brief Compute the distance between two 3D vectors
 * @param a first vector
 * @param b second vector
 * @return distance in Q16.16
 */
i32 vec3_dist(const Vec3 *a, const Vec3 *b);

/**
 * @brief Normalize a 3D vector
 * @param v input vector
 * @param out output vector
 */
void vec3_normalize(const Vec3 *v, Vec3 *out);

/**
 * @brief Print a vector4
 * @param v pointer to the vector
 */
void vec4_print(const Vec4 *v);

/**
 * @brief Add two vector4s
 * @param a first vector
 * @param b second vector
 * @param out output vector
 */
void vec4_add(const Vec4 *a, const Vec4 *b, Vec4 *out);

/**
 * @brief Subtract two vector4s
 * @param a first vector
 * @param b second vector
 * @param out output vector
 */
void vec4_sub(const Vec4 *a, const Vec4 *b, Vec4 *out);

/**
 * @brief Scale a vector4 by a scalar
 * @param v input vector
 * @param s scalar value
 * @param out output vector
 */
void vec4_scale(const Vec4 *v, i32 s, Vec4 *out);

/**
 * @brief Computes the squared length of given vector.
 * @param v Input vector
 * @return squared length in Q16.16
 */
i32 vec4_length_sq(const Vec4 *v);

/**
 * @brief Computes the length of given vector.
 * @param v Input vector
 * @return 32-bit integer length.
 */
i32 vec4_length(const Vec4 *v);

/**
 * @brief Computes squared distance between two POSITION vectors.
 * @param a Input vector a.
 * @param b Input vector b.
 * @return squared distance in Q16.16
 */
i32 vec4_dist_sq(const Vec4 *a, const Vec4 *b);

/**
 * @brief Computes distance between two POSITION vectors.
 * @param a Input vector a.
 * @param b Input vector b.
 * @return 32-bit integer distance (Q16.16).
 */
i32 vec4_dist(const Vec4 *a, const Vec4 *b);

/**
 * @brief Compute the dot product of two vector4s
 * @param a first vector
 * @param b second vector
 * @return dot product
 */
i32 vec4_dot(const Vec4 *a, const Vec4 *b);

/**
 * @brief Compute the cross product of two vector4s
 * @param a first vector
 * @param b second vector
 * @param out output vector
 */
void vec4_cross(const Vec4 *a, const Vec4 *b, Vec4 *out);

/**
 * @brief Compute the cross product of the xyz components of two Vec4s; w is set to 0
 * @param a first vector
 * @param b second vector
 * @param out output vector (w = FX_ONE)
 */
void vec4_cross3(const Vec4 *a, const Vec4 *b, Vec4 *out);

/**
 * @brief Normalize a vector4
 * @param v input vector
 * @param out output vector
 */
void vec4_normalize(const Vec4 *v, Vec4 *out);

/**
 * @brief Normalizes the (x, y, z) components of a vec4 while preserving w.
 * If the vector length is zero, it returns the original vector.
 * @param v input vector
 * @param out output vector
 */
void vec4_normalize3(const Vec4 *v, Vec4 *out);

/**
 * @brief Linearly interpolate between two Vec4 world-space positions (w cleared).
 * @param a start vector
 * @param b end vector
 * @param t interpolation factor Q16.16 (0..FX_ONE)
 * @param out output vector
 */
void vec4_lerp(const Vec4 *a, const Vec4 *b, i32 t, Vec4 *out);

/**
 * @brief Bilinearly interpolate a quad's four world-space corners (w cleared).
 * Corner convention: v[0]=TL(u=0,t=0), v[1]=TR(u=1,t=0), v[2]=BR(u=1,t=1), v[3]=BL(u=0,t=1).
 * @param v array of four corner Vec4s
 * @param u horizontal parameter Q16.16 (0..FX_ONE)
 * @param t vertical parameter Q16.16 (0..FX_ONE)
 * @param out output interpolated Vec4
 */
void vec4_bilerp(const Vec4 v[4], i32 u, i32 t, Vec4 *out);

#endif /* FIXED_H */