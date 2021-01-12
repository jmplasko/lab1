CC = gcc
CFLAGS = -g -Wall -ansi -pedantic -D_POSIX_C_SOURCE
LD = gcc
LDFLAGS =
all: mush
mush: Functions.c mush.c
	$(CC) $(CFLAGS) -o mush Functions.c mush.c
