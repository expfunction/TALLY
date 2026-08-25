/* Copyright (c) 2026 Burak Yazar */

/* CAN vertex animation format (v2) for CVGA engine.
 *
 * Quake-style per-vertex keyframe animation: each frame stores the complete
 * deformed vertex array. The engine lerps between adjacent frames at runtime.
 * Supports multiple named clips referencing slices of a shared frame pool.
 *
 * Binary layout
 * -------------
 * CANHeader      16 bytes
 * CANClipDir[]   nclips * 16 bytes
 * FrameTimes[]   nframes * 4 bytes   (i32 Q16.16, seconds from clip start)
 * VertexData     nframes * nvertices * 12 bytes  (3 x i32 Q16.16 per vertex)
 */
#ifndef CAN_H
#define CAN_H

#include "../CORE/TYPES/MESH.H"
#include "../RNDR/ANIM.H"

#define MAX_CAN_CLIPS 8
#define MAX_CAN_FRAMES 512
#define MAX_CAN_VERTS 4096

#pragma pack(push, 1)
typedef struct CANHeader_s
{
    u8 magic[4]; /* "CAN2" or "CAN3" */
    u8 nclips;
    u8 nslots;
    u16 nframes;
    u16 nvertices;
    u32 ofs_slots;
    u32 ofs_events;
    u16 nevents;
} CANHeader;
#pragma pack(pop)

typedef struct CANEvent_s
{
    u16 frame;     /* Frame index in the global pool */
    u16 type;      /* Event type (1 = SOUND) */
    char name[12]; /* Event parameter/name (e.g., sound name) */
} CANEvent;

typedef struct CANClipDir_s
{
    char name[8]; /* null-padded */
    u16 first;    /* first frame index in pool */
    u16 count;    /* number of frames          */
    u8 mode;      /* default AnimMode          */
    u8 reserved[3];
} CANClipDir;

/** @brief One animation clip: a named slice of the shared frame pool. */
typedef struct CANClip_s
{
    char name[8];  /* null-padded 8-char clip name                    */
    u16 first;     /* index of first frame in pool (integer)          */
    u16 count;     /* number of frames, minimum 2 (integer)           */
    i32 duration;  /* = times[first + count - 1], Q16.16 seconds      */
    AnimMode mode; /* default playback mode                           */
} CANClip;

/**
 * @brief Loaded vertex-animation file.
 *
 * Vertex layout (flat array):
 *   verts[(frame * nvertices + vertex) * 3 + axis]  axis: 0=x 1=y 2=z
 *
 * Frame times are relative to the clip start (times[clip.first] = 0).
 * Call can_free() to release heap storage.
 */
typedef struct CANFile_s
{
    CANClip clips[MAX_CAN_CLIPS];
    i32 *times; /* heap: nframes x i32 Q16.16 seconds      */
    i32 *verts; /* heap: nframes x nvertices x 3 x i32     */
    MeshSlot *slots;
    CANEvent *events;
    u16 nevents;
    u16 nclips;
    u16 nframes;
    u16 nvertices; /* must match MeshCMS.vertex_count          */
    u8 nslots;
} CANFile;

/** @brief Load a .CAN file into *out. Returns 1 on success, 0 on failure. */
int can_load(const char *path, CANFile *out);

/** @brief Free heap storage allocated by can_load. */
void can_free(CANFile *cf);

/**
 * @brief Find a clip by name (case-insensitive, 8-char max).
 * @return Clip index [0..nclips-1], or -1 if not found.
 */
int can_find(const CANFile *cf, const char *name);

/**
 * @brief Find a slot by name.
 * @return 1 if found, 0 if not.
 */
int can_get_slot(const CANFile *cf, const char *name, MeshSlot *out_slot);

#endif /* CAN_H */
