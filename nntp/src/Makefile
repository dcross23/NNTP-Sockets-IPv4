.PHONY: all
all: 
	gcc -g server/server.c server/server.h -o ../bin/serv
	gcc -g client/client_tcp.c -o ../bin/ct
	gcc -g client/client_udp.c -o ../bin/cu

.PHONY: run	
run: all
	../bin/serv

.PHONY: stop	
stop: 
	pkill serv

.PHONY: see	
see:
	lsof -i:7692

.PHONY: clean
clean: stop
	rm ../bin/*
