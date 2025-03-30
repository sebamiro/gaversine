NAME=gaversine
GENERATOR=generator

CFLAGS=-std=c99 -Wall -Werror -Wextra -O2
LFLAGS=-lm

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	CFLAGS += -D _GNU_SOURCE
endif

all:
	gcc $(CFLAGS) main.c $(LFLAGS) -o $(NAME)

profile: CFLAGS += -DPROFILE

profile: all

generator: generator.c
	gcc $(CFLAGS) generator.c $(LFLAGS) -o $(GENERATOR)

repread: test_repeat_read.c repeater.c
	gcc $(CFLAGS) test_repeat_read.c $(LFLAGS) -o $(@)
