/* Copyright (c) 2026 Burak Yazar */

#ifndef FAI_H
#define FAI_H

#include "CORE\TYPES.H"

#define MAP_WIDTH 24
#define MAP_HEIGHT 24
#define MAP_GRID (MAP_WIDTH * MAP_HEIGHT)

extern u8 g_map_grid[MAP_GRID];

typedef struct Node
{
    int x, y;
    int g_cost;
    int h_cost;
    int f_cost;
    int is_obstacle;
    int visited; // used for closed list
    int in_open; // used for open list
    struct Node *parent;
} Node;

/**
 * @brief Initiate Astar nodes using loaded level string
 * @param level_string
 * @return 0: error 1: success
 */
int fai_init_map(const u8 *level_string);
void fai_update_map(const char *grid);
Node *fai_find_path(int start_x, int start_y, int end_x, int end_y);
void fai_set_obstacle(int x, int y, int is_obstacle);
int fai_has_world_los(const Vec4 *from, const Vec4 *to);

#endif // FAI_H