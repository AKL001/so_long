#include "mlx/minilibx-linux/mlx.h"
#include <stdio.h>
#include <stdlib.h>

#define TILE_SIZE 44
#define ESC 65307
#define A 97
#define S 115
#define D 100
#define W 119
#define CHAR_SIZE 40 
#define GAME_WIDTH 1000
#define GAME_HIGHT 500

typedef enum e_direction {
    DOWN,
    UP,
    LEFT,
    RIGHT,
    IDLE
} t_direction;

typedef struct s_assets {
    void    *floor_img;
    void    *player_img;
    void    *wall_img;
}           t_assets;

typedef struct s_img
{
    void    *img;
    char    *addr;
    int     bpp;
    int     line_len;
    int     endian;
}              t_img;

typedef struct s_player_state {
    int         moving;
    int         frame;
    int         sleep;
    int         offset_x;
    int         offset_y;
    t_direction direction;
    t_direction last_direction;
}           t_player_state;

typedef struct s_game {
    void    *mlx;
    void    *win;
    int     **map;
    int     player_x;
    int     player_y;
    int     map_height;
    int     map_width;
    t_assets assets;
    t_img   player_img;
    t_img   buffer;
    t_player_state player_state;
    int     needs_render;
}       t_game;

int get_pixel_color(t_img *img, int x, int y)
{
    char *pixel;

    if (!img || !img->addr)
    {
        printf("Error: Invalid image or address\n");
        return 0;
    }

    if (x < 0 || y < 0 || x >= CHAR_SIZE || y >= CHAR_SIZE)
    {
        printf("Warning: Out of bounds pixel access (%d, %d)\n", x, y);
        return 0;
    }

    pixel = img->addr + (y * img->line_len + x * (img->bpp / 8));
    return *(unsigned int *)pixel;
}

void put_pixel_to_buffer(t_img *buffer, int x, int y, int color)
{
    char *dst;

    dst = buffer->addr + (y * buffer->line_len + x * (buffer->bpp / 8));
    *(unsigned int *)dst = color;
}

void load_image(t_game *game, char *path)
{
    int width, height;
    void *new_img;

    new_img = mlx_xpm_file_to_image(game->mlx, path, &width, &height);
    if (!new_img)
    {
        printf("Error loading image: %s\n", path);
        exit(1);
    }

    if (game->player_img.img)
        mlx_destroy_image(game->mlx, game->player_img.img);

    game->player_img.img = new_img;
    game->player_img.addr = mlx_get_data_addr(game->player_img.img, 
        &game->player_img.bpp, &game->player_img.line_len, &game->player_img.endian);
}

void map_init(t_game *game)
{
    int x, y = 0;
    const char *map_layout[] = {
        "1111111111111111111111111111111",
        "100000P000000000000100000000001",
        "1000000000000000000000000000001",
        "1000000000000000000000000000001",
        "1000000000000000000000000000001",
        "1000000000000000000000000000001",
        "1000000000000000000000000000001",
        "1111111111111111111111111111111",
        NULL
    };

    game->map_height = 8;
    game->map_width = 31;
    game->map = malloc(game->map_height * sizeof(int *));
    if (!game->map)
        exit(1);
    while (y < game->map_height)
    {
        game->map[y] = malloc(game->map_width * sizeof(int));
        if (!game->map[y])
            exit(1);
        x = 0;
        while (x < game->map_width)
        {
            if (map_layout[y][x] == 'P')
            {
                game->player_x = x;
                game->player_y = y;
                game->map[y][x] = 2;
            }
            else
                game->map[y][x] = map_layout[y][x] - '0';
            x++;
        }
        y++;
    }
}

const char *get_direction_frame(t_direction dir, int frame)
{
    static const char *frames[4][4] = {
        {
            "assets/character/Down1.xpm",
            "assets/character/Down2.xpm",
            "assets/character/Down3.xpm",
            "assets/character/Down4.xpm"
        },
        {
            "assets/character/Top1.xpm",
            "assets/character/Top2.xpm",
            "assets/character/Top3.xpm",
            "assets/character/Top4.xpm"
        },
        {
            "assets/character/Left1.xpm",
            "assets/character/Left2.xpm",
            "assets/character/Left3.xpm",
            "assets/character/Left4.xpm"
        },
        {
            "assets/character/Right1.xpm",
            "assets/character/Right2.xpm",
            "assets/character/Right3.xpm",
            "assets/character/Right4.xpm"
        }
    };

    if (dir >= 0 && dir < 4)
        return frames[dir][frame % 4];
    return frames[0][0];
}

void render_player(t_game *game)
{
    t_player_state *state = &game->player_state;
    const char *frame_path;
    
    if (state->moving)
        frame_path = get_direction_frame(state->direction, state->frame);
    else
        frame_path = get_direction_frame(state->last_direction, 0);
    
    load_image(game, (char *)frame_path);
    
    int draw_x = state->moving ? state->offset_x : game->player_x * TILE_SIZE;
    int draw_y = state->moving ? state->offset_y : game->player_y * TILE_SIZE;
    
    int offset = (TILE_SIZE - CHAR_SIZE) / 2;
    mlx_put_image_to_window(game->mlx, game->win, game->player_img.img, 
        draw_x + offset, draw_y + offset);
}

void render_map(t_game *game)
{
    int x, y = 0;

    while (y < game->map_height)
    {
        x = 0;
        while (x < game->map_width)
        {
            if (game->map[y][x] == 1)
                mlx_put_image_to_window(game->mlx, game->win, game->assets.wall_img, 
                    x * TILE_SIZE, y * TILE_SIZE);
            else if (game->map[y][x] == 0 || game->map[y][x] == 2)
                mlx_put_image_to_window(game->mlx, game->win, game->assets.floor_img, 
                    x * TILE_SIZE, y * TILE_SIZE);
            x++;
        }
        y++;
    }
}

int update_movement(t_game *game)
{
    t_player_state *state = &game->player_state;
    
    if (!game->needs_render && !state->moving)
        return 0;

    if (state->moving && state->sleep-- <= 0)
    {
        state->sleep = 5;
        state->frame = (state->frame + 1) % 4;
    }

    if (state->moving)
    {
        int range = TILE_SIZE / 4;
        int target_x = game->player_x * TILE_SIZE;
        int target_y = game->player_y * TILE_SIZE;
        
        if (state->offset_x < target_x)
            state->offset_x += range;
        else if (state->offset_x > target_x)
            state->offset_x -= range;
            
        if (state->offset_y < target_y)
            state->offset_y += range;
        else if (state->offset_y > target_y)
            state->offset_y -= range;

        if (state->offset_x == target_x && state->offset_y == target_y)
        {
            state->moving = 0;
            state->last_direction = state->direction;
        }
    }

    render_map(game);
    render_player(game);
    game->needs_render = 0;

    return 0;
}

int keys(int keycode, t_game *game)
{
    if (keycode == ESC)
    {
        mlx_destroy_window(game->mlx, game->win);
        exit(0);
    }

    if (!game->player_state.moving)
    {
        int new_x = game->player_x;
        int new_y = game->player_y;
        t_direction new_direction = game->player_state.direction;

        if (keycode == W) { new_y--; new_direction = UP; }
        if (keycode == S) { new_y++; new_direction = DOWN; }
        if (keycode == A) { new_x--; new_direction = LEFT; }
        if (keycode == D) { new_x++; new_direction = RIGHT; }

        if (new_x >= 0 && new_x < game->map_width &&
            new_y >= 0 && new_y < game->map_height &&
            game->map[new_y][new_x] != 1)
        {
            game->player_x = new_x;
            game->player_y = new_y;
            
            game->player_state.moving = 1;
            game->player_state.direction = new_direction;
            game->player_state.frame = 0;
            game->player_state.offset_x = game->player_x * TILE_SIZE;
            game->player_state.offset_y = game->player_y * TILE_SIZE;
            game->needs_render = 1;
        }
    }

    return 0;
}

void game_init(t_game *game)
{
    int height, width;

    game->player_state.moving = 0;
    game->player_state.frame = 0;
    game->player_state.sleep = 5;
    game->player_state.offset_x = 0;
    game->player_state.offset_y = 0;
    game->player_state.direction = DOWN;
    game->player_state.last_direction = DOWN;
    game->needs_render = 1;

    game->assets.floor_img = mlx_xpm_file_to_image(game->mlx, "assets/floors/floor.xpm", &width, &height);
    if (!game->assets.floor_img)
    {
        printf("error floor\n");
        exit(1);
    }

    game->assets.wall_img = mlx_xpm_file_to_image(game->mlx, "assets/walls/wall.xpm", &width, &height);
    if (!game->assets.wall_img)
    {
        printf("error wall\n");
        exit(1);
    }

    game->buffer.img = mlx_new_image(game->mlx, GAME_WIDTH, GAME_HIGHT);
    if (!game->buffer.img)
    {
        printf("Error creating buffer image\n");
        exit(1);
    }

    game->buffer.addr = mlx_get_data_addr(game->buffer.img, &game->buffer.bpp, 
        &game->buffer.line_len, &game->buffer.endian);
    if (!game->buffer.addr)
    {
        printf("Error getting buffer address\n");
        exit(1);
    }

    game->player_img.img = NULL;
    load_image(game, "assets/character/Down1.xpm");
}

int main()
{
    t_game game;

    game.mlx = mlx_init();
    game.win = mlx_new_window(game.mlx, GAME_WIDTH, GAME_HIGHT, "test");

    game_init(&game);
    map_init(&game);
    render_map(&game);
    
    mlx_hook(game.win, 2, 1L << 0, keys, &game);
    mlx_loop_hook(game.mlx, update_movement, &game);
    mlx_loop(game.mlx);
    
    return 0;
}
