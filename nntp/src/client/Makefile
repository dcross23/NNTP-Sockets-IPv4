CC=gcc
CFLAGS=-g -Wunused

CLIENTS=clients

.PHONY: client
client: client.c client_tcp.o client_udp.o ../params.h
	$(CC) $(CFLAGS) $^ -o ../../bin/client
	
	
client_tcp.o: $(CLIENTS)/client_tcp.c $(CLIENTS)/client_tcp.h ../params.h
	gcc -c $(CLIENTS)/client_tcp.c
	
client_udp.o: $(CLIENTS)/client_udp.c $(CLIENTS)/client_udp.h ../params.h
	gcc -c $(CLIENTS)/client_udp.c
	
	
.PHONY:clean
clean:
	rm *.o

