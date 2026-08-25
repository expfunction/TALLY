/* Copyright (c) 2026 Burak Yazar */

#ifndef PRTCL_H
#define PRTCL_H

#include "../CORE/TYPES/WORLD.H"
#include "../CORE/TYPES/GFX.H"
#include "../RNDR/CAMER.H"

/* Maximum particles per bucket. */
#define BUCKET_CAP MAX_PARTICLES

typedef struct PrtclScreenPt_s
{
	u32 offset; /* byte offset = y * MODEX_PITCH_BYTES + (sx >> 2) */
	Vec2 screen_pos;
	u32 depth;
	u8 color;
} PrtclScreenPt;

/**
 * @brief Initialize particle system. Call once at startup.
 */
void prtcl_init(void);

/**
 * @brief Update all emitters and particles.
 * @param dt delta time Q16.16
 */
void prtcl_update(i32 dt);

/**
 * @brief Render all active particles as single pixels with plane batching.
 * @param buf  VRAM base pointer
 * @param cam  camera for world-to-screen transform
 * @param num_lights  number of world lights
 * @param lights  point light array
 * @param clip  clipping rectangle
 */
void prtcl_render(u8 *buf, const Camera *cam,
				  int num_lights, const PointLight *lights,
				  const ClipRect *clip);

/**
 * @brief Get the global emitter array (for editor / serialization).
 * @return pointer to emitter array [MAX_EMITTERS]
 */
ParticleEmitter *prtcl_get_emitters(void);

/**
 * @brief Get number of active emitters.
 */
int prtcl_get_num_emitters(void);

/**
 * @brief Activate a new emitter from the pool.
 * @param em emitter definition to copy in
 * @return emitter index, or -1 if pool full
 */
int prtcl_add_emitter(const ParticleEmitter *em);

/**
 * @brief Deactivate and clear an emitter.
 * @param idx emitter index
 */
void prtcl_remove_emitter(int idx);

/**
 * @brief Kill all particles (but keep emitters). Useful on level load.
 */
void prtcl_clear_particles(void);

/**
 * @brief Manually spawn a single particle (for one-shot effects / entity use).
 * @param pos   world position
 * @param vel   initial velocity
 * @param life  lifetime Q16.16
 * @param color palette index
 * @return particle index, or -1if pool full
 */
int prtcl_spawn_one(const Vec4 *pos, const Vec3 *vel, i32 life, u8 color, u8 color_end, i32 damping, i16 material_id);

#endif /* PRTCL_H */