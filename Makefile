NAME=parser

all:
	gcc -DDEBUG -Wall -Werror -Wextra main.c -o $(NAME)

