#ifndef FLEVEL_H
#define FLEVEL_H

#include "CORE/CYBER.H"

void flevel_init(void);
void flevel_load(const char *mapName);
void level_swap_wall_texture(const char *matName, const char *texName);
int flevel_is_in_warm_light(Vec4 position);
void flevel_update(void);
void flevel_draw(Camera *cam, Surface8 *surf, const ClipRect *clip_rect);

#endif
