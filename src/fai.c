/* Copyright (c) 2026 Burak Yazar */

#include "fai.h"
#include "CORE\CONSL.H"
#include <math.h>
#ifdef PLATFORM_WIN64
#include <corecrt_math.h>
#endif
#include <stdlib.h>

// Definition of the global map grid
u8 g_map_grid[MAP_GRID];

// Node grid to hold pathfinding state
static Node s_nodes[MAP_HEIGHT][MAP_WIDTH];

// Open list
static Node *s_open_list[MAP_GRID];
static int s_open_list_count = 0;

/**
 * @brief Initiates global map grid for Astar
 * @param level linear string of defined level
 * @return  0: error 1: success
 */
static int fai_load_map_grid(const u8 *level)
{
    int i;
    if (!level)
    {
        console_log("Error loading level for Astar!");
        return 0;
    }

    for (i = 0; i < MAP_GRID; i++)
    {
        g_map_grid[i] = level[i];
    }
    return 1;
}

int fai_init_map(const u8 *level_string)
{
    int x, y;
    if (!level_string)
    {
        console_log("Error loading level for Astar!");
        return 0;
    }

    fai_update_map((const char *)level_string);

    for (y = 0; y < MAP_HEIGHT; y++)
    {
        for (x = 0; x < MAP_WIDTH; x++)
        {
            s_nodes[y][x].x = x;
            s_nodes[y][x].y = y;
            s_nodes[y][x].is_obstacle = g_map_grid[y * MAP_WIDTH + x] ? 1 : 0;
            s_nodes[y][x].g_cost = 0;
            s_nodes[y][x].h_cost = 0;
            s_nodes[y][x].f_cost = 0;
            s_nodes[y][x].visited = 0;
            s_nodes[y][x].in_open = 0;
            s_nodes[y][x].parent = NULL;
        }
    }
    return 1;
}

void fai_update_map(const char *grid)
{
    int i, x = 0, y = 0;
    if (!grid)
        return;

    for (i = 0; grid[i] != '\0'; i++)
    {
        char c = grid[i];

        if (c == '\n' || c == '\r')
            continue;

        if (x < MAP_WIDTH && y < MAP_HEIGHT)
        {
            // Define static obstacles
            int is_obstacle = 0;
            if (c == '#' || c == '/' || c == '$' || c == '~' || c == '=' || c == '%' || c == '^' || c == ':')
            {
                is_obstacle = 1;
            }
            g_map_grid[y * MAP_WIDTH + x] = is_obstacle;
        }

        x++;
        if (x >= MAP_WIDTH)
        {
            x = 0;
            y++;
        }
    }
}

void fai_set_obstacle(int x, int y, int is_obstacle)
{
    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT)
    {
        g_map_grid[y * MAP_WIDTH + x] = is_obstacle;
        s_nodes[y][x].is_obstacle = is_obstacle;
    }
}

// Helper to calculate heuristic (Manhattan distance)
static int calculate_h_cost(int x1, int y1, int x2, int y2)
{
    return abs(x1 - x2) + abs(y1 - y2);
}

// Add node to open list
static void add_to_open_list(Node *node)
{
    s_open_list[s_open_list_count++] = node;
    node->in_open = 1;
}

// Remove node from open list
static void remove_from_open_list(int index)
{
    s_open_list[index]->in_open = 0;
    s_open_list_count--;
    if (index < s_open_list_count)
    {
        s_open_list[index] = s_open_list[s_open_list_count];
    }
}

static void find_closest_free_node(int *x, int *y, int target_x, int target_y)
{
    if (*x >= 0 && *x < MAP_WIDTH && *y >= 0 && *y < MAP_HEIGHT)
    {
        if (!g_map_grid[*y * MAP_WIDTH + *x])
            return; // Already free!
    }

    int best_x = *x, best_y = *y;
    int min_dist = 999999;

    int dx[8] = {0, 0, -1, 1, -1, 1, -1, 1};
    int dy[8] = {-1, 1, 0, 0, -1, -1, 1, 1};

    for (int i = 0; i < 8; i++)
    {
        int nx = *x + dx[i];
        int ny = *y + dy[i];
        if (nx >= 0 && nx < MAP_WIDTH && ny >= 0 && ny < MAP_HEIGHT)
        {
            if (!g_map_grid[ny * MAP_WIDTH + nx])
            {
                int dist = abs(nx - target_x) + abs(ny - target_y);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    best_x = nx;
                    best_y = ny;
                }
            }
        }
    }

    *x = best_x;
    *y = best_y;
}

Node *fai_find_path(int start_x, int start_y, int end_x, int end_y)
{
    int i, x, y;
    int best_index;
    int nx, ny;
    int step_cost, new_g, new_h, new_f;
    Node *current_node;
    Node *start_node;
    Node *end_node;
    Node *child;

    // Up, Down, Left, Right
    int dx[4] = {0, 0, -1, 1};
    int dy[4] = {-1, 1, 0, 0};

    // Bounds check for start and end
    if (start_x < 0 || start_x >= MAP_WIDTH || start_y < 0 || start_y >= MAP_HEIGHT)
        return NULL;
    if (end_x < 0 || end_x >= MAP_WIDTH || end_y < 0 || end_y >= MAP_HEIGHT)
        return NULL;

    // Reset nodes state before search
    for (y = 0; y < MAP_HEIGHT; y++)
    {
        for (x = 0; x < MAP_WIDTH; x++)
        {
            s_nodes[y][x].is_obstacle = g_map_grid[y * MAP_WIDTH + x] ? 1 : 0;
            s_nodes[y][x].g_cost = 0;
            s_nodes[y][x].h_cost = 0;
            s_nodes[y][x].f_cost = 0;
            s_nodes[y][x].visited = 0;
            s_nodes[y][x].in_open = 0;
            s_nodes[y][x].parent = NULL;
        }
    }

    // Resolve nearest free tiles if start or end are marked obstacle
    find_closest_free_node(&start_x, &start_y, end_x, end_y);
    find_closest_free_node(&end_x, &end_y, start_x, start_y);

    if (g_map_grid[start_y * MAP_WIDTH + start_x] || g_map_grid[end_y * MAP_WIDTH + end_x])
        return NULL;

    start_node = &s_nodes[start_y][start_x];
    end_node = &s_nodes[end_y][end_x];

    if (start_x == end_x && start_y == end_y)
        return start_node;

    s_open_list_count = 0;

    // Add start node to open list
    add_to_open_list(start_node);

    while (s_open_list_count > 0)
    {
        // Get the node with least f_cost
        best_index = 0;
        for (i = 1; i < s_open_list_count; i++)
        {
            if (s_open_list[i]->f_cost < s_open_list[best_index]->f_cost)
            {
                best_index = i;
            }
        }

        current_node = s_open_list[best_index];
        remove_from_open_list(best_index);

        current_node->visited = 1; // Add to closed list

        // Found the goal
        if (current_node->x == end_x && current_node->y == end_y)
        {
            return current_node; // Success! Backtrack using parent pointers
        }

        // Generate children (adjacent nodes)
        for (i = 0; i < 4; i++)
        {
            nx = current_node->x + dx[i];
            ny = current_node->y + dy[i];

            // Bounds check
            if (nx < 0 || ny < 0 || nx >= MAP_WIDTH || ny >= MAP_HEIGHT)
                continue;

            child = &s_nodes[ny][nx];

            // Obstacle check
            if (child->is_obstacle)
                continue;

            // Child is in closed list
            if (child->visited)
                continue;

            step_cost = 1;
            new_g = current_node->g_cost + step_cost;
            new_h = calculate_h_cost(nx, ny, end_x, end_y);
            new_f = new_g + new_h;

            // Child is already in open list
            if (child->in_open)
            {
                if (new_g >= child->g_cost)
                    continue;
            }

            // Update child
            child->g_cost = new_g;
            child->h_cost = new_h;
            child->f_cost = new_f;
            child->parent = current_node;

            // Add the child to the open list if not already there
            if (!child->in_open)
            {
                add_to_open_list(child);
            }
        }
    }

    // No path found
    return NULL;
}

int fai_has_world_los(const Vec4 *from, const Vec4 *to)
{
    if (!from || !to)
        return 0;

    Vec4 diff;
    vec4_sub(to, from, &diff);
    diff.y = 0;
    i32 total_dist = vec4_length(&diff);
    if (total_dist == 0)
        return 1;

    const i32 step_size = FX_FROM_FLOAT(0.35f);
    int num_steps = FX_TO_INT(fx_div_q16(total_dist, step_size));
    if (num_steps < 1)
        num_steps = 1;

    Vec4 step_vec;
    vec4_normalize3(&diff, &step_vec);
    step_vec.x = fx_mul_q16(step_vec.x, step_size);
    step_vec.z = fx_mul_q16(step_vec.z, step_size);

    Vec4 curr = *from;

    for (int s = 1; s <= num_steps; s++)
    {
        curr.x += step_vec.x;
        curr.z += step_vec.z;

        int gx = (int)floorf(FX_TO_FLOAT(curr.x) / 2.0f + 0.5f);
        int gz = (int)floorf(FX_TO_FLOAT(curr.z) / 2.0f + 0.5f);

        if (gx < 0 || gx >= MAP_WIDTH || gz < 0 || gz >= MAP_HEIGHT)
            return 0;

        if (g_map_grid[gz * MAP_WIDTH + gx])
            return 0;
    }

    return 1;
}
