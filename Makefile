CC = gcc

CFLAGS = -Wall -Wextra -Werror

theshell: src/main.c
	$(CC) $(CFLAGS) src/main.c -o theshell

#and if I want to run like this i do
run: theshell
	./theshell

val: theshell
	valgrind ./theshell

clean: 
	rm -f theshell

