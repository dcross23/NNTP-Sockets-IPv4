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

#include "../params.h"

#define MAXHOST 512

extern int errno;


/*
 *			H A N D L E R
 *
 *	This routine is the signal handler for the alarm signal.
 */
void handler()
{
	printf("Alarma recibida \n");
}

/*
 *			M A I N
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
int main(int argc, char** argv)
{
	int i, errcode;
	int s;				/* socket descriptor */
	long timevar;                   /* contains time returned by time() */
	struct sockaddr_in myaddr_in;	/* for local socket address */
	struct sockaddr_in servaddr_in;	/* for server socket address */
	int	addrlen, n_retry;
	struct sigaction vec;
	struct addrinfo hints, *res;
	
	char command[COMMAND_SIZE];	/* This example uses COMMAND_SIZE byte messages. */
	char tmp[COMMAND_SIZE];
	FILE *commandsFile;		/* File that contains client NNTP commands to be executed */

	if (argc != 2) {
		fprintf(stderr, "Usage:  %s <nameserver>\n", argv[0]);
		exit(1);
	}
	
	/* Create the socket. */
	s = socket (AF_INET, SOCK_DGRAM, 0);
	if (s == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to create socket\n", argv[0]);
		exit(1);
	}
	
	/* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));
	
	
	/* Bind socket to some local address so that the
	 * server can send the reply back.  A port number
	 * of zero will be used so that the system will
	 * assign any available port number.  An address
	 * of INADDR_ANY will be used so we do not have to
	 * look up the internet address of the local host.
	 */
	myaddr_in.sin_family = AF_INET;
	myaddr_in.sin_port = 0;
	myaddr_in.sin_addr.s_addr = INADDR_ANY;
	if (bind(s, (const struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to bind socket\n", argv[0]);
		exit(1);
	}
	
	
	addrlen = sizeof(struct sockaddr_in);
	if (getsockname(s, (struct sockaddr *)&myaddr_in, &addrlen) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to read socket address\n", argv[0]);
		exit(1);
	}

	/* Print out a startup message for the user. */
	time(&timevar);
	
	/* The port number must be converted first to host byte
	* order before printing.  On most hosts, this is not
	* necessary, but the ntohs() call is included here so
	* that this program could easily be ported to a host
	* that does require it.
	*/
    	printf("[UDP] \"False connected\" to %s on port %u at %s", 
    		argv[1], ntohs(myaddr_in.sin_port), (char *) ctime(&timevar));

	/* Set up the server address. */
	servaddr_in.sin_family = AF_INET;
	
	/* Get the host information for the server's hostname that the
	 * user passed in.
	 */
	memset (&hints, 0, sizeof (hints));
	hints.ai_family = AF_INET;
	
	
	/* esta función es la recomendada para la compatibilidad con IPv6 gethostbyname queda obsoleta*/
	errcode = getaddrinfo (argv[1], NULL, &hints, &res); 
	if (errcode != 0){
		/* Name was not found.  Return a
		 * special value signifying the error. */
		fprintf(stderr, "%s: No es posible resolver la IP de %s\n",
				argv[0], argv[1]);
		exit(1);
	}
	else {
		/* Copy address of host */
		servaddr_in.sin_addr = ((struct sockaddr_in *) res->ai_addr)->sin_addr;
	}

	freeaddrinfo(res);
	
	/* PORT del servidor en orden de red*/
	servaddr_in.sin_port = htons(PORT);


   	/* Registrar SIGALRM para no quedar bloqueados en los recvfrom */
	vec.sa_handler = (void *) handler;
	vec.sa_flags = 0;
	if ( sigaction(SIGALRM, &vec, (struct sigaction *) 0) == -1) {
		perror(" sigaction(SIGALRM)");
		fprintf(stderr,"%s: unable to register the SIGALRM signal\n", argv[0]);
		exit(1);
	}
	
	
	
	/* Send a "false conexion" message to the UDP server listening socket (ls_UDP) */
	if (sendto (s, " ", 1,0, (struct sockaddr *)&servaddr_in, addrlen) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to send request to \"connect\" \n", argv[0]);
		exit(1);
	}	
	
	/* Waits for the response of the server with the new socket it has to talk to */
	n_retry = RETRIES;
	while(n_retry > 0){
		alarm(TIMEOUT);
		if (recvfrom (s, tmp, 1, 0, (struct sockaddr *)&servaddr_in, &addrlen) == -1) {
			if (errno == EINTR) {
				 /* Need to retry the request if we have
				 * not already exceeded the retry limit.
				 */
	 		     fprintf(stderr,"Alarm went off.\n");
	 		     n_retry--; 
			} else  {
				fprintf(stderr,"Unable to get response to \"connect\"\n");
				exit(1); 
			}
		}else{
			alarm(0);
			break;
		}
	}
	
	if(n_retry == 0){ 
		exit(1);
	}
	
	
	
//---------
	commandsFile = fopen("../src/client/someNNTPCommands.txt", "r");
	if(commandsFile == NULL){
		fprintf(stderr, "Cannot read NNTP commands file\n");
		exit(1);
	}	
	
	while( fgets(command, sizeof(command), commandsFile) != NULL){
		//Sends command to server
		if (sendto(s, command, COMMAND_SIZE, 0, (struct sockaddr *)&servaddr_in, addrlen) == -1) {
			fprintf(stderr, "%s: Connection aborted on error ", argv[0]);
			fprintf(stderr, "on send number %d\n", i);
			exit(1);
		}
		
		printf("[UDP] Command Readed: %s", command);	
		
		//Receive response from the server
		n_retry = RETRIES;
		while(n_retry > 0){
			alarm(TIMEOUT);
			if (recvfrom (s, command, COMMAND_SIZE, 0, (struct sockaddr *)&servaddr_in, &addrlen) == -1) {
				if (errno == EINTR) {
		 		     fprintf(stderr,"Alarm went off.\n");
		 		     n_retry--; 
				} else  {
					fprintf(stderr,"Unable to get response to \"connect\"\n");
					exit(1); 
				}
			}else{
				alarm(0);
				// Print out message indicating the identity of this reply. 
				printf("[UDP] Command scanned by the server: %s\n\n", command);	
				
				break;
			}
		}
		
		if(n_retry == 0){ 
			exit(1);
		}
	}

	fclose(commandsFile);

//---------

 /* Print message indicating completion of task. */
	time(&timevar);
	printf("[UDP] All done at %s", (char *)ctime(&timevar));
}
