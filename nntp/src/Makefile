CC=gcc
CFLAGS=-g -Wunused


.PHONY: all
all:
	cd server && make server
	cd client && make client
	

.PHONY: server
server: all
	../bin/server

.PHONY: ct
ct:
	../bin/client localhost TCP

.PHONY: cu
cu:
	../bin/client localhost UDP



.PHONY: stop	
stop: 
	pkill server


.PHONY: see	
see:
	lsof -i:7692



.PHONY: clean
clean: 
	rm ../bin/*
	cd client && make clean
