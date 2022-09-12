CFLAGS  = -g -Wall
CC = gcc

TARGETS = client_program server_program

all: $(TARGETS)

client_program: client/uftp_client.c
	gcc client/uftp_client.c -o client/client

server_program: server/uftp_server.c
	gcc server/uftp_server.c -o server/server

clean:
	rm client/client server/server