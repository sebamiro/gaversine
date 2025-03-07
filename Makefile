NAME=parser

all:
	gcc -std=c99 -DDEBUG -Wall -Werror -Wextra main.c -o $(NAME)

