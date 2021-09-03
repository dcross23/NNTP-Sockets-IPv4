CC=gcc
CFLAGS=-g -Wunused

CLIENTS=clients

.PHONY: client
client: client.c client_tcp.o client_udp.o params.h client_commands.o
	$(CC) $(CFLAGS) $^ -o ../../bin/client
	
	
client_tcp.o: $(CLIENTS)/client_tcp.c $(CLIENTS)/client_tcp.h params.h client_commands.o
	gcc -c $(CLIENTS)/client_tcp.c
	
client_udp.o: $(CLIENTS)/client_udp.c $(CLIENTS)/client_udp.h params.h client_commands.o
	gcc -c $(CLIENTS)/client_udp.c
	
client_commands.o: client_commands.c client_commands.h
	gcc -c client_commands.c
	
.PHONY:clean
clean:
	rm *.o

