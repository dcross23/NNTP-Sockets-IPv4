CC=gcc
CFLAGS=-g -Wunused

.PHONY: server
server: server.c server.h ../params.h server_commands.c server_commands.h
	$(CC) $(CFLAGS) $^ -o ../../bin/server
	
