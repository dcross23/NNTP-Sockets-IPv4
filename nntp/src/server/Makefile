CC=gcc
CFLAGS=-g -Wunused

.PHONY: server
server: server.c server.h ../params.h
	$(CC) $(CFLAGS) server.c server.h -o ../../bin/server
	
.PHONY: clean
clean:
	rm cUDP.txt cTCP.txt
