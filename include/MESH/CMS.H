/* Copyright (c) 2026 Burak Yazar */

/* CMS mesh format for CVGA engine */
#ifndef CMS_H
#define CMS_H

#include "../CORE/TYPES/MESH.H"

/* CMS file header (0x30 = 48 bytes).
 *
 * Version history:
 *   1 - original; submesh material_id indexed m_textures[] directly
 *   2 - submesh material_id now indexes the global material library
 *       (m_mat_*[]) introduced with the material system. Byte layout
 *       unchanged; auto-seeded material slots mirror texture slots so
 *       v1 files keep working without migration. */
typedef struct CMSHeader_s
{
    u8 magic[4];     /* "CMS1" */
    u16 version;     /* 1 or 2 */
    u8 flags;        /* CVG_HAS_UV | CVG_HAS_NORMALS | CVG_HAS_VCOLORS */
    u8 coord_format; /* 0 = i32 Q16.16 positions */
    u16 vertex_count;
    u16 index_count;
    u16 submesh_count;
    u16 slot_count;
    u32 ofs_vertices;
    u32 ofs_indices;
    u32 ofs_uv0;
    u32 ofs_normals;
    u32 ofs_vcolors;
    u32 ofs_submeshes;
    i32 scale_fx; /* Q16.16 overall scale */
    u32 ofs_slots;
} CMSHeader;

/* API */

/**
 * @brief Loads a CMS(CyberMesh) mesh from file. Note: there won't be 1 suffix in filenames
 * @param path CMS file path
 * @param out_mesh Out mesh pointer
 * */
int load_cms(const char *path, MeshCMS *out_mesh); /* returns 1 on success */

/**
 * @brief Load per-mesh textures from TEXTURES/ subfolder next to the CMS file
 * @param cms_path Full path to the .CMS file
 * @param m Pointer to mesh (must be loaded)
 * @return 1 if any textures loaded, 0 otherwise
 */
int load_mesh_textures(const char *cms_path, MeshCMS *m);

/**
 * @brief Load per-mesh animations from ANIMS/ subfolder next to the CMS file
 * @param can_path Full path to the .CAN file
 * @param m Pointer to mesh (must be loaded)
 * @return 1 if any animations loaded, 0 otherwise
 */
int load_mesh_animations(const char *can_path, MeshCMS *m);

/**
 * @brief Frees mesh allocation
 * @param m Pointer to mesh object
 */
void free_mesh(MeshCMS *m);

/**
 * @brief Get a slot by name from a mesh.
 * @param m Pointer to mesh object
 * @param name Name of the slot to find
 * @param out_slot Receives the slot definition
 * @return 1 if found, 0 if not
 */
int mesh_get_slot(const MeshCMS *m, const char *name, MeshSlot *out_slot);

/**
 * @brief Resolve a flat clip index (across all loaded CAN files) to file and
 *        local clip indices. Sets m->cva to the resolved file as a side-effect.
 * @param m        Mesh with loaded CAN files
 * @param flat_idx Flat clip index (0..total_clips-1)
 * @param out_fi   Receives file index into cva_files[]
 * @param out_ci   Receives local clip index within that file
 * @return 1 on success, 0 if flat_idx out of range
 */
int mesh_resolve_clip(MeshCMS *m, int flat_idx, int *out_fi, int *out_ci);

#endif /* CMS_H */
