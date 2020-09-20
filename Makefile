snake: snake.c
	cc -g -std=c99 -lncurses $< -o $@

.PHONY: install
install: snake
	mkdir -p ${out}/bin
	install snake ${out}/bin/snake
