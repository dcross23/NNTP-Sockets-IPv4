CC=gcc
CFLAGS=-g -Wunused


.PHONY: all
all:
	cd server && make server
	cd client && make client
	
.PHONY: stop	
stop: 
	pkill server


.PHONY: see	
see:
	lsof -i:7692



.PHONY: clean
clean: 
	git restore ../noticias
	rm ../bin/server ../bin/client
	rm ../bin/logs/*.txt ../bin/logs/*.log
	cd client && make clean
