CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c11

all: clone_a1 clone_a2

clone_a1: clone_a1.c clone_a1.c
	${CC} ${CFLAGS} clone_a1.c -o clone_a1

clone_a2: clone_a2.c clone_a2.c
	${CC} ${CFLAGS} clone_a2.c -o clone_a2

clean:
	rm -f clone_a1 clone_a2

.PHONY: clean all