#include "mlx/minilibx-linux/mlx.h"
#include <stdio.h>
#include <stdlib.h>
#include <X11/X.h>
#include <X11/keysym.h>

#include <string.h> // For strlen and strchr

typedef struct s_data
{
	void *mlx_ptr;
	void *win_ptr;
	void *floor_img;
	void *tree_img;
	int img_width;
	int img_height;	
	int map_width;
	int map_height;
} t_data;

// Function to calculate map dimensions
void calculate_map_dimensions(const char *map, int *width, int *height)
{
	char *copy = strdup(map); // Copy to avoid modifying original
	char *line = strtok(copy, "\n");

	*height = 0;
	*width = 0;

	while (line)
	{
		int line_length = strlen(line);
		if (line_length > *width)
			*width = line_length; // Find the max line length
		(*height)++; // Count lines
		line = strtok(NULL, "\n");
	}
	free(copy); // Free allocated memory
}

// Function to put image at (x, y)
void put_img(t_data *data, int x, int y, void *img)
{
	mlx_put_image_to_window(data->mlx_ptr, data->win_ptr, img, x, y);
}

int	main(void)
{
	t_data	data;
	char map[] =
		"1111111111111111111111111111111111\n"
		"1000000000000000000000000000000001\n"
		"1010010100100000101001000000010101\n"
		"1010010010101010001001000000010101\n"
		"1000000000000000000000000000000001\n"
		"1000000000000000000000000000000001\n"
		"1000000000000000000000000000000001\n"
		"1000000000000000000000000000000001\n"
		"1000000000000000000000000000000001\n"
		"1000000000000000000000000000000001\n"
		"1000000000000000000000000000000001\n"
		"1111111111111111111111111111111111";

	char *floor_path = "src/floor4.xpm";
	char *tree_path = "src/wall.xpm";

	// Initialize MiniLibX
	data.mlx_ptr = mlx_init();
	if (!data.mlx_ptr) { printf("Error: MiniLibX initialization failed.\n"); return 1; }

	// Calculate map dimensions safely
	calculate_map_dimensions(map, &data.map_width, &data.map_height);
	printf("Map dimensions: %d x %d\n", data.map_width, data.map_height);

	// Load images
	data.floor_img = mlx_xpm_file_to_image(data.mlx_ptr, floor_path, &data.img_width, &data.img_height);
	if (!data.floor_img) { printf("Error loading floor image.\n"); return 1; }

	data.tree_img = mlx_xpm_file_to_image(data.mlx_ptr, tree_path, &data.img_width, &data.img_height);
	if (!data.tree_img) { printf("Error loading tree image.\n"); return 1; }

	// Create window
	int window_width = data.map_width * data.img_width;
	int window_height = data.map_height * data.img_height;
	data.win_ptr = mlx_new_window(data.mlx_ptr, window_width, window_height, "Dynamic Map");
	if (!data.win_ptr) { printf("Error creating window.\n"); return 1; }

	// Parse map and render images
	char *map_copy = strdup(map); // Copy for safe tokenization
	char *line = strtok(map_copy, "\n");
	int y = 0;

	while (line)
	{
		int x = 0;
		for (int i = 0; line[i]; i++)
		{
			// Always draw the floor first
			put_img(&data, x, y, data.floor_img);

			// If it's a '1', draw the tree on top of the floor
			if (line[i] == '1')
				put_img(&data, x, y, data.tree_img);

			x += data.img_width;
		}
		y += data.img_height;
		line = strtok(NULL, "\n");
	}

	free(map_copy); // Free the allocated memory

	// Keep window open
	mlx_loop(data.mlx_ptr);

	return 0;
}