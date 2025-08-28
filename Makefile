CC = gcc

# Target: all (default target)
all: server client

server: server.o
	$(CC) bin/socket.o bin/server.o -o build/server

server.o: server.c socket.o
	$(CC) -c server.c -o bin/server.o

client: client.o
	$(CC) bin/socket.o bin/client.o -o build/client

client.o: client.c socket.o
	$(CC) -c client.c -o bin/client.o

socket.o:
	$(CC) -c socket/socket.c -o bin/socket.o
