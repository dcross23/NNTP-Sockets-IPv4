#ifndef __CLIENT_TCP_H__
#define __CLIENT_TCP_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <time.h>

#include "../../params.h"

/*
 * Groups the action of recieving a message using TCP connection. This is calling
 *  recv function and waiting in case there is more data arrieving
 */
int recvTCP(int s, char *response, int size);


/*
 *			C L I E N T   T C P
 *
 *	This routine is the client which request service from the remote.
 *	It creates a connection, sends a number of
 *	requests, shuts down the connection in one direction to signal the
 *	server about the end of data, and then receives all of the responses.
 *	Status will be written to stdout.
 *
 *	The name of the system to which the requests will be sent is given
 *	as a parameter to the command.
 */
int clienttcp(char** argv);


#endif
