snake: snake.c
	cc -g -std=c99 -lncurses $< -o $@
