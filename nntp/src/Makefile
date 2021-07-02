CC=gcc
CFLAGS=-g -Wunused


.PHONY: all
all: 
	$(CC) $(CFLAGS) server/server.c server/server.h -o ../bin/serv
	$(CC) $(CFLAGS) client/client_tcp.c -o ../bin/ct
	$(CC) $(CFLAGS) client/client_udp.c -o ../bin/cu

.PHONY: run	
run: all
	../bin/serv

.PHONY: ct
ct:
	../bin/ct localhost

.PHONY: stop	
stop: 
	pkill serv

.PHONY: see	
see:
	lsof -i:7692

.PHONY: clean
clean: stop
	rm ../bin/*
