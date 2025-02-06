#include <stdlib.h>
#include <stdio.h>
#include "../mlx/minilibx-linux/mlx.h"

typedef struct	s_data {
	void	*img;
	char	*img_pixels_ptr;
	int		bits_per_pixel;
	int		line_length;
	int		endian;
}				t_data;

typedef struct s_var 
{
    void    *mlx;
    void    *win;
    t_data  img;
}              t_var;

void	my_mlx_pixel_put(t_data *img, int x, int y, int color)
{
    int offset;
    offset = (y * img->line_length + x * (img->bits_per_pixel / 8));
    *(unsigned int *)(img->img_pixels_ptr + offset) = color;
}

void  render_xpm_image(t_var *data, char *file, int pos_x, int pos_y)
{
    t_data xpm;
    int x, y;
    int *xpm_pixels;
    int width = 44, height = 44;  // Fix spelling

    // Load XPM image
    xpm.img = mlx_xpm_file_to_image(data->mlx, file, &width, &height);
    if (!xpm.img)
    {
        fprintf(stderr, "Error: Failed to load XPM file %s\n", file);
        return;
    }

    // Get pixel data from the XPM image
    xpm.img_pixels_ptr = mlx_get_data_addr(xpm.img, &xpm.bits_per_pixel, &xpm.line_length, &xpm.endian);
    xpm_pixels = (int *)xpm.img_pixels_ptr;

    // Correctly loop through `width x height`
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int color = xpm_pixels[y * (xpm.line_length / (xpm.bits_per_pixel / 8)) + x];  // Fix pixel index calculation

            if (color != 0x000000) // Skip black (assuming transparency)
                my_mlx_pixel_put(&data->img, x + pos_x, y + pos_y, color);
        }
    }

    // Display the updated image
    mlx_put_image_to_window(data->mlx, data->win, data->img.img, 0, 0);
}

int main()
{
    t_var vars;

    vars.mlx = mlx_init();
    vars.win = mlx_new_window(vars.mlx, WIN_WIDTH, WIN_WIDTH, "XPM Renderer");

    vars.img.img = mlx_new_image(vars.mlx, WIN_WIDTH, WIN_WIDTH);  // Full window image buffer
    vars.img.img_pixels_ptr = mlx_get_data_addr(vars.img.img, &vars.img.bits_per_pixel, &vars.img.line_length, &vars.img.endian);

    // Render the XPM image at (50, 50)
    render_xpm_image(&vars, "assets/Tree.xpm", 50, 50);

    mlx_loop(vars.mlx);
}
