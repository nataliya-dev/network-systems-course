CFLAGS  = -g -Wall -pthread
CC = gcc

TARGETS = client_program server_program

all: $(TARGETS)

client_program: src/client.c
	gcc $(CFLAGS) src/client.c -o client

server_program: src/server.c
	gcc $(CFLAGS) src/server.c -o server

clean:
	rm client server