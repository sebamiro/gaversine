NAME=gaversine
GENERATOR=generator

CFLAGS=-std=c99 -Wall -Werror -Wextra
LFLAGS=-lm

all:
	gcc $(CFLAGS) main.c $(LFLAGS) -o $(NAME)

generator: generator.c
	gcc $(CFLAGS) generator.c $(LFLAGS) -o $(GENERATOR)

