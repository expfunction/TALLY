#ifndef LEVEL_H
#define LEVEL_H

#include "CORE/CYBER.H"

void level_init(void);
void level_load(const char *mapName);
void level_swap_wall_texture(const char *matName, const char *texName);
int level_is_in_warm_light(Vec4 position);
void level_update(void);
void level_draw(Camera *cam, Surface8 *surf);

#endif
