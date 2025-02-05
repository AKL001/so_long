#include <stdlib.h>
#include <stdio.h>
#include "mlx/minilibx-linux/mlx.h"

// #define ESC 53
// #define W 13
// #define A 0
// #define S 1
// #define D 2

#define ESC 65307
#define W 119
#define A 97
#define S 115
#define D 100
typedef struct s_vars
{
    void    *mlx;
    void    *win;
    void    *floor_img;
    void    *wall_img;
    void    *char_img;
    int     img_width;
    int     img_height;
    int     box_x;
    int     box_y;
    char    **map;
    int     move_x;
    int     move_y;
    int     moving;
    int     step;
    int    *char_data; // Pixel data for character
} t_vars;


char *map[] = {
    "1111111111111111111111111111111111111111111111111111111111111",
    "1001000000000000000000000000000000000000000000000000000000011",
    "1000101001000001010010000000101000000000000000000000000000011",
    "1000110101010100010010000000101000000000000000000000000000001",
    "1000001000000000000000000000000000000000000000000000000000001",  // 'P' indicates player spawn
    "1000000100000000000000000000000000000000000000000000000000001",
    "1000000010000000000000000000000000000000000000000000000000001",
    "1000000001000000000000000000000000000000000000000000000000001",
    "1000000000100000000000000000000000000000000000000000000000001",
    "100000000001000000000000000000000000000000000000000000000001",
    "1100000P0001000000000000000000000000000000000000000000000001",
    "111111111111111111111111111111111111111111111111111111111111",
    NULL
};
void extract_xpm_data(t_vars *vars)
{
    int i;
    int *data;
    int bpp, size_line, endian;

    data = (int *)mlx_get_data_addr(vars->char_img, &bpp, &size_line, &endian);
    vars->char_data = malloc(vars->img_width * vars->img_height * sizeof(int));
    if (!vars->char_data)
    {
        fprintf(stderr, "Memory allocation failed for pixel data\n");
        exit(1);
    }
    i = 0;
    while (i < vars->img_width * vars->img_height)
    {
        vars->char_data[i] = data[i]; // Copy pixel data
        i++;
    }
}

void render_character(t_vars *vars, int x, int y)
{
    int i, j, color;

    for (i = 0; i < vars->img_height; i++)
    {
        for (j = 0; j < vars->img_width; j++)
        {
            color = vars->char_data[i * vars->img_width + j];

            if ((unsigned int)color != 0xFF000000) // Ignore transparent pixels
                mlx_pixel_put(vars->mlx, vars->win, x + j, y + i, color);
        }
    }
}

int keys(int keycode, t_vars *vars)
{
    if (keycode == ESC)
    {
        mlx_destroy_window(vars->mlx, vars->win);
        free(vars);
        exit(1);
    }

    if (!vars->moving) // Only move if not already in motion
    {
        int tile_x = vars->box_x / 44;
        int tile_y = vars->box_y / 44;

        if (keycode == W && map[tile_y - 1][tile_x] != '1') { vars->move_y = -1; vars->move_x = 0; vars->moving = 1; }
        if (keycode == A && map[tile_y][tile_x - 1] != '1') { vars->move_x = -1; vars->move_y = 0; vars->moving = 1; }
        if (keycode == S && map[tile_y + 1][tile_x] != '1') { vars->move_y = 1; vars->move_x = 0; vars->moving = 1; }
        if (keycode == D && map[tile_y][tile_x + 1] != '1') { vars->move_x = 1; vars->move_y = 0; vars->moving = 1; }
    }

    return 0;
}

void render_map(t_vars *vars)
{
    int x, y;
    int tile_size = 44;

    for (y = 0; map[y]; y++)
    {
        for (x = 0; map[y][x]; x++)
        {
            void *img = (map[y][x] == '1') ? vars->wall_img : vars->floor_img;
            mlx_put_image_to_window(vars->mlx, vars->win, img, x * tile_size, y * tile_size);
        }
    }
}
int update_movement(t_vars *vars)
{
    if (!vars->moving)
        return 0;
    render_map(vars);
    // Convert pixel position to tile coordinates
    int old_tile_x = vars->box_x / 44;
    int old_tile_y = vars->box_y / 44;
    int new_tile_x = (vars->box_x + vars->move_x) / 44;
    int new_tile_y = (vars->box_y + vars->move_y) / 44;

    // Restore previous tile before moving
    // void *background_img = (map[old_tile_y][old_tile_x] == '1') ? vars->wall_img : vars->floor_img;
    // mlx_put_image_to_window(vars->mlx, vars->win, background_img, vars->box_x, vars->box_y);

    // Collision check: Prevent moving into walls
    if (map[new_tile_y][new_tile_x] == '1')
    {
        vars->moving = 0;
        return 0; // Stop movement
    }

    // Move pixel by pixel
    vars->box_x += vars->move_x;
    vars->box_y += vars->move_y;

    // Draw the character at new position
    // mlx_put_image_to_window(vars->mlx, vars->win, vars->floor_img, vars->box_x, vars->box_y);
    render_character(vars, vars->box_x, vars->box_y);

    // Stop moving when reaching a full tile
    vars->step++;
    if (vars->step >= 44)
    {
        vars->moving = 0;
        vars->move_x = 0;
        vars->move_y = 0;
        vars->step = 0;
    }

    return 0;
}

void find_player_spawn(t_vars *vars)
{
    int x, y;
    int tile_size = 44; // Tile size should match movement grid

    for (y = 0; map[y]; y++)
    {
        for (x = 0; map[y][x]; x++)
        {
            if (map[y][x] == 'P')
            {
                mlx_put_image_to_window(vars->mlx, vars->win, vars->floor_img, x, y);
                vars->box_x = x * tile_size;
                vars->box_y = y * tile_size;
                return; // Stop searching after finding 'P'
            }
        }
    }
}
int render_game(t_vars *vars)
{
    render_map(vars);
    // Redraw only the tile beneath the character to maintain background
    mlx_put_image_to_window(vars->mlx, vars->win, vars->floor_img, vars->box_x, vars->box_y);

    // Draw the player character at new position
    mlx_put_image_to_window(vars->mlx, vars->win, vars->char_img, vars->box_x, vars->box_y);

    mlx_do_sync(vars->mlx);
    return 1;
}
int main()
{
    t_vars *vars = malloc(sizeof(t_vars));
    if (!vars)
    {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }

    vars->mlx = mlx_init();
    vars->win = mlx_new_window(vars->mlx, 1200, 800  , "HELLO");

    vars->floor_img = mlx_xpm_file_to_image(vars->mlx, "src/floor4.xpm", &vars->img_width, &vars->img_height);
    vars->wall_img = mlx_xpm_file_to_image(vars->mlx, "src/wall.xpm", &vars->img_width, &vars->img_height);
    vars->char_img = mlx_xpm_file_to_image(vars->mlx, "src/Tree2.xpm", &vars->img_width, &vars->img_height);

    if (!vars->floor_img || !vars->wall_img || !vars->char_img)
    {
        fprintf(stderr, "Error: Failed to load image\n");
        return 1;
    }

    find_player_spawn(vars); // Find and set player's start position
    extract_xpm_data(vars); // Convert character to pixel data

    render_game(vars);

    mlx_key_hook(vars->win, keys, vars);
    mlx_loop_hook(vars->mlx, update_movement, vars); // Continuous movement update
    mlx_loop(vars->mlx);

    free(vars);
    return 0;
}