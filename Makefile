NAME=gaversine
GENERATOR=generator

JSON=./gaversine_13245_10000000.json
DATA=./gaversine_13245_10000000.data

CFLAGS=-std=c99 -Wall -Werror -Wextra
LFLAGS=-lm

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	CFLAGS += -D _GNU_SOURCE
endif

all:
	gcc $(CFLAGS) main.c $(LFLAGS) -o $(NAME)

profile: CFLAGS += -DPROFILE
profile: all

run:
	./$(NAME) $(JSON) $(DATA)

generator: generator.c
	gcc $(CFLAGS) generator.c $(LFLAGS) -o $(GENERATOR)

repread: test_repeat_read.c repeater.c
	gcc $(CFLAGS) test_repeat_read.c $(LFLAGS) -o $(@)
