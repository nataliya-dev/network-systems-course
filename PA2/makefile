CFLAGS  = -g -Wall -pthread
CC = gcc

TARGETS = server_program

all: $(TARGETS)

server_program: src/server.c
	gcc $(CFLAGS) src/server.c -o server

clean:
	rm src/server