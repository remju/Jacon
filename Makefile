CC = gcc
CFLAGS = -Wall -Wextra -ggdb

all: jacon

jacon: jacon.c jacon.h
	$(CC) $(CFLAGS) -o jacon jacon.c

clean:
	rm -f jacon