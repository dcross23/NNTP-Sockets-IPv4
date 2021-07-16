#ifndef __CLIENT_UDP_H__
#define __CLIENT_UDP_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../../params.h"


#define MAXHOST 512

extern int errno;


/*
 * Groups the action of recieving a message using TCP conexion. This is calling
 *  recv function and waiting in case there is more data arrieving
 */
int recvUDP(int s, char *response, int size, struct sockaddr_in *servaddr_in, int *addrlen);


/*
 *			H A N D L E R
 *
 *	This routine is the signal handler for the alarm signal.
 */
void handler();

/*
 *			C L I E N T   U D P
 *
 *	This routine is the client which requests service from the remote
 *	"example server".  It will send a message to the remote nameserver
 *	requesting the internet address corresponding to a given hostname.
 *	The server will look up the name, and return its internet address.
 *	The returned address will be written to stdout.
 *
 *	The name of the system to which the requests will be sent is given
 *	as the first parameter to the command.  The second parameter should
 *	be the the name of the target host for which the internet address
 *	is sought.
 */
int clientudp(char **argv);


#endif
