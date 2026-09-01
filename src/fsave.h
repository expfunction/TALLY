/* Copyright (c) 2026 Burak Yazar */

#ifndef FSAVE_H
#define FSAVE_H

#include "fgame.h"

int fsave_checkpoint(void);
int fsave_load(void);
int fsave_exists(void);
void fsave_delete(void);
void fsave_invalidate_cache(void);
LevelID fsave_get_saved_level(void);

#endif
 
