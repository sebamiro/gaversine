NAME=gaversine
GENERATOR=generator

JSON=./gaversine_13245_10000000.json
DATA=./gaversine_13245_10000000.data

CFLAGS=-std=c99 -Wall -Werror -Wextra -O2
LFLAGS=-lm

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	CFLAGS += -D _GNU_SOURCE
endif

all:
	gcc $(CFLAGS) main.c $(LFLAGS) -o $(NAME)
	@echo OK

profile: CFLAGS += -DPROFILE
profile: all

debug: CFLAGS += -g
debug: all

run: all
	./$(NAME) $(JSON) $(DATA)

generator: generator.c
	gcc $(CFLAGS) generator.c $(LFLAGS) -o $(GENERATOR)

repread:
	gcc $(CFLAGS) test_repeat_read.c $(LFLAGS) -o $(@)
	./$(@) $(JSON)
.PHONY: repread

clean:
	rm -rf $(GENERATOR) $(NAME) repread
