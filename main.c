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



void render_pixel_by_pixel(t_game *game, int px, int py)
{
    int x, y, color,offset;

    if (!game->player_img.addr || !game->buffer.addr)
    {
        printf("Error: Images not properly initialized\n");
        return;
    }

    // Draw character
    for (y = 0; y < CHAR_SIZE; y++)
    {
        for (x = 0; x < CHAR_SIZE; x++)
        {
            color = get_pixel_color(&game->player_img, x, y);
            if (color != 0xFF000000) 
            {
                put_pixel_to_buffer(&game->buffer, px + x, py + y, color);
            }
        }
    }
    offset = (TILE_SIZE - CHAR_SIZE) / 2;
    mlx_put_image_to_window(game->mlx, game->win, game->player_img.img, px + offset, py + offset);
    
}

void load_image(t_game *game, char *path)
{
    int width, height;

    game->player_img.img = mlx_xpm_file_to_image(game->mlx, path, &width, &height);
    if (!game->player_img.img)
    {
        printf("Error loading image: %s\n", path);
        exit(1);
    }

    game->player_img.addr = mlx_get_data_addr(game->player_img.img, &game->player_img.bpp, &game->player_img.line_len, &game->player_img.endian);
    if (!game->player_img.addr)
    {
        printf("Error getting data address for image: %s\n", path);
        exit(1);
    }
}



void map_init(t_game *game)
{
    int x , y = 0;
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
    game->map = malloc(game->map_height * (sizeof(int *)));
    if(!game->map)
        exit(1);
    while(y < game->map_height)
    {  
        game->map[y] = malloc(game->map_width * sizeof(int));
        if(!game->map[y])
            exit(1);
        x = 0;
        while(x < game->map_width)
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
    // game->player_x = 2;
    // game->player_y = 2;
}

void    render_map(t_game *game)
{
    int x, y = 0;

    mlx_destroy_image(game->mlx, game->buffer.img);
    game->buffer.img = mlx_new_image(game->mlx, GAME_WIDTH, GAME_HIGHT);
    game->buffer.addr = mlx_get_data_addr(game->buffer.img, &game->buffer.bpp, &game->buffer.line_len, &game->buffer.endian);

    while (y < game->map_height)
    {
        x = 0;
        while (x < game->map_width)
        {
            if (game->map[y][x] == 1)
                mlx_put_image_to_window(game->mlx, game->win, game->assets.wall_img, x * TILE_SIZE, y * TILE_SIZE);
            else if (game->map[y][x] == 0 || game->map[y][x] == 2)
                mlx_put_image_to_window(game->mlx, game->win, game->assets.floor_img, x * TILE_SIZE, y * TILE_SIZE);
            x++;
        }
        y++;
    }
    render_pixel_by_pixel(game, game->player_x * TILE_SIZE, game->player_y * TILE_SIZE);

    // mlx_put_image_to_window(game->mlx, game->win, game->buffer.img, 0, 0);
}

int keys(int keycode, t_game *game)
{
    int new_x = game->player_x;
    int new_y = game->player_y;

    if (keycode == ESC)
    {
        mlx_destroy_window(game->mlx, game->win);
        exit(0);
    }

    if (keycode == W) new_y--;  // Move up
    if (keycode == S) new_y++;  // Move down
    if (keycode == A) new_x--;  // Move left
    if (keycode == D) new_x++;  // Move right

    // Check if new position is within the map and is a floor tile (0)
    if (new_x >= 0 && new_x < game->map_width &&
        new_y >= 0 && new_y < game->map_height &&
        game->map[new_y][new_x] != 1)
    {
        // Clear previous player position by redrawing floor tile
        mlx_put_image_to_window(game->mlx, game->win, game->assets.floor_img, 
            game->player_x * TILE_SIZE, game->player_y * TILE_SIZE);

        // Update player position
        game->player_x = new_x;
        game->player_y = new_y;

        // Re-render the player in the new position
        render_pixel_by_pixel(game, game->player_x * TILE_SIZE, game->player_y * TILE_SIZE);
    }

    return (0);
}


void game_init(t_game *game)
{
    int hight, width;

    // floor xpm to img
    game->assets.floor_img = mlx_xpm_file_to_image(game->mlx,"assets/floors/floor.xpm",&width,&hight);
    if(!game->assets.floor_img)
    {
        printf("error floor\n");
        exit(1);
    }
    // wall xpm to img 
    game->assets.wall_img = mlx_xpm_file_to_image(game->mlx,"assets/walls/wall.xpm",&width,&hight);
    if(!game->assets.wall_img)
    {
        printf("error wall\n");
        exit(1);
    }
    // do we need this ??? player xpm to image
    game->assets.player_img = mlx_xpm_file_to_image(game->mlx, "assets/character/Down1.xpm", &width, &hight);
    if (!game->assets.player_img)
    {
        printf("error loading player image\n");
        exit(1);
    }
    game->buffer.img = mlx_new_image(game->mlx, GAME_WIDTH, GAME_HIGHT);
    if (!game->buffer.img)
    {
    printf("Error creating buffer image\n");
    exit(1);
    }
    game->buffer.addr = mlx_get_data_addr(game->buffer.img, &game->buffer.bpp, &game->buffer.line_len, &game->buffer.endian);
    if (!game->buffer.addr)
    {
    printf("Error getting buffer address\n");
    exit(1);
    }

    load_image(game,"assets/character/Down1.xpm");

}



int main()
{
    t_game game;

    game.mlx = mlx_init();
    game.win = mlx_new_window(game.mlx,1000,500,"test");

    game_init(&game);
    map_init(&game);
    render_map(&game);
    mlx_hook(game.win, 2, 1L << 0, keys, &game);

    // mlx_key_hook(game.mlx,keys,&game);
    // render_pixel_by_pixel(&game,100,100);

    mlx_loop(game.mlx);


}
