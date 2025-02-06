1- we can use mlx_xpm_file_to_image; for the character to not lose backgroun
2- or use "mlx_xpm_to_image" ;
3- "mlx_destroy_image" ??? 
4- "mlx_get_screen_size"
5- use -> void	my_mlx_pixel_put(t_data *img, int x, int y, int color)
{
    int offset;
    offset = (y * img->line_length + x * (img->bits_per_pixel / 8));
    *(unsigned int *)(img->img_pixels_ptr + offset) = color;
}
