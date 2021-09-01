#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <stdbool.h>

#define MAXHOST 	128
#define RESP_200(code) (code >= 200 && code < 300)
#define RESP_300(code) (code >= 300 && code < 400)
#define RESP_400(code) (code >= 400 && code < 500)
#define RESP_500(code) (code >= 500 && code < 600)

extern int errno;

int checkCommand(char *command);
int addNewConexionToLog(struct sockaddr_in servaddr_in, struct sockaddr_in clientaddr_in, char *protocol);
int addCommandToLog(char *command, bool isResponse);

void serverTCP(int s, struct sockaddr_in peeraddr_in);
void serverUDP(int s, struct sockaddr_in clientaddr_in);

void errout(char *);		/* declare error out routine */

#endif
