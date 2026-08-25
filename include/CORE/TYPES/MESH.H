/* Copyright (c) 2026 Burak Yazar */

#ifndef MESH_TYPES_H
#define MESH_TYPES_H

#include "MATH.H"

#define MAX_MESH_TEXTURES 16
#define MAX_CAN_FILES 8

/* Forward declaration — full definition in MESH/CAN.H */
struct CANFile_s;

typedef struct MeshSlot_s
{
    char name[14];
    u16 vertex_idx;
} MeshSlot;

typedef struct MeshCMS_s
{
    u16 vertex_count;
    u16 index_count;
    u16 submesh_count;
    u16 slot_count;

    MeshSlot *slots;   /* array of slot_count elements */
    Vec4 *pos_fx;      /* v*3, Q16.16 world-space positions */
    Vec2 *uv_fx;       /* v*2, Q16.16 texture coordinates (or NULL) */
    Vec4 *norm_fx;     /* v*3, Q16.16 vertex normals (or NULL) */
    u8 *vcolors;       /* v*1 palette index (or NULL) */
    u16 *indices;      /* i */
    u16 *submesh_data; /* sm*4 (first,count,mat,0) */

    Mat4x4 rotation; /* Q16.16 rotation matrix */
    i32 scale_fx;    /* uniform scale Q16.16 (FX_ONE = 1.0) */
    u8 flags;

    u8 *textures[MAX_MESH_TEXTURES];
    u16 texture_w[MAX_MESH_TEXTURES];
    u16 texture_count;

    struct CANFile_s *cva_files[MAX_CAN_FILES];
    u8 cva_nfiles;
    struct CANFile_s *cva;
    Vec4 *cva_scratch;
    Vec4 *cva_trans;
    Vec4 *cva_prev; /* previous-frame positions for velocity buffer */
} MeshCMS;

#endif /* MESH_TYPES_H */
