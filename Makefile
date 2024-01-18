CC = gcc
CFLAGS = -Wall -Werror -Wpedantic -D_POSIX_SOURCE -Wextra -std=c11 -D_GNU_SOURCE

all: clone_a1 clone_a2

clone_a1: clone_a1.c clone_a1.c
	${CC} ${CFLAGS} clone_a1.c -o clone_a1

clone_a2: clone_a2.c clone_a2.c
	${CC} ${CFLAGS} clone_a2.c -o clone_a2

clean:
	rm -f clone_a1 clone_a2

.PHONY: clean all