NAME=gaversine
GENERATOR=generator

CFLAGS=-std=c99 -Wall -Werror -Wextra

all:
	gcc $(CFLAGS) main.c -o $(NAME)

generator: generator.c
	gcc $(CFLAGS) generator.c -o $(GENERATOR)

