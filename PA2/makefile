CFLAGS  = -g -Wall
CC = gcc

TARGETS = client_program server_program

all: $(TARGETS)

client_program: src/client.c
	gcc src/client.c -o src/client

server_program: src/server.c
	gcc -pthread src/server.c -o src/server

clean:
	rm src/client src/server