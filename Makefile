NAME = so_long

SRC = so_long.c

INCLUDES = libmlx_Linux.a
CC = gcc
CCFLAGS = -Wextra -Wall -Werror

OBJ = $(SRC:.c=.o)

# Compile the game 
all: $(NAME)

$(NAME): $(OBJ) 
	$(CC) so_long.c $(INCLUDES) -Imlx_linux -lXext -lX11 -lm -lz 
# $(NAME): $(OBJ)
# 	$(CC) $(OBJ) -Lmlx_linux  -L/usr/lib -Imlx_linux -lXext -lX11 -lm -lz -o $(NAME)

# Compile .c files into .o

# %.o:%.c
# 	cc -Wall -Wextra -Werror  -c $< -o $@
%.o: %.c
	$(CC) -I/usr/include -Imlx_linux -O3 -c $< -o $@

# Clean object files
clean:
	rm -f $(OBJ)

# Full clean including the executable
fclean: clean
	rm -f $(NAME)

# Recompile everything
re: fclean all
