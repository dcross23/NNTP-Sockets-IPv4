#include "server.h"
#include "../params.h"


bool END_LOOP = false;          
void endProgram(){ END_LOOP = true; }


void handler()
{
	printf("[SERV] Alarma recibida \n");
}


/*
 *			M A I N
 *
 *	This routine starts the server.  It forks, leaving the child
 *	to do all the work, so it does not have to be run in the
 *	background.  It sets up the sockets.  It
 *	will loop forever, until killed by a signal.
 *
 */
int main(int argc, char **argv)
{

	int s_TCP, s_UDP;		/* connected socket descriptor */
	int ls_TCP, ls_UDP;		/* listening socket descriptor */

	int br;		 		/* contains the number of bytes read */
     
	struct sigaction sa = {.sa_handler = SIG_IGN}; /* used to ignore SIGCHLD */
    
	struct sockaddr_in myaddr_in;		/* for local socket address */
	struct sockaddr_in clientaddr_in;	/* for peer socket address */
	int addrlen;
	
	fd_set readmask;
	int numfds,s_bigger;

	char buffer[COMMAND_SIZE];		/* buffer for packets to be read into */

	struct sigaction vec;

	
	/* Register SIGALARM */
	vec.sa_handler = (void *) handler;
	vec.sa_flags = 0;
	if ( sigaction(SIGALRM, &vec, (struct sigaction *) 0) == -1) {
		perror(" sigaction(SIGALRM)");
		fprintf(stderr,"%s: unable to register the SIGALRM signal\n", argv[0]);
		exit(1);
	}	
	
	
	/* Create the listen TCP socket. */
	ls_TCP = socket (AF_INET, SOCK_STREAM, 0);
	if (ls_TCP == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to create socket TCP\n", argv[0]);
		exit(1);
	}
	
	/* Clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
   	memset ((char *)&clientaddr_in, 0, sizeof(struct sockaddr_in));

    	addrlen = sizeof(struct sockaddr_in);

	/* Set up address structure for the listen socket. */
	myaddr_in.sin_family = AF_INET;
		/* The server should listen on the wildcard address,
		 * rather than its own internet address.  This is
		 * generally good practice for servers, because on
		 * systems which are connected to more than one
		 * network at once will be able to have one server
		 * listening on all networks at once.  Even when the
		 * host is connected to only one network, this is good
		 * practice, because it makes the server program more
		 * portable.
		 */
	myaddr_in.sin_addr.s_addr = INADDR_ANY;
	myaddr_in.sin_port = htons(PORT);


	/* Bind the listen address to the socket. */
	if (bind(ls_TCP, (const struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to bind address TCP\n", argv[0]);
		exit(1);
	}
	
	/* Initiate the listen on the socket so remote users
	 * can connect.  The listen backlog is set to 5, which
	 * is the largest currently supported.
	 */
	if (listen(ls_TCP, 5) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to listen on socket\n", argv[0]);
		exit(1);
	}
	
	
	/* Create the socket UDP. */
	ls_UDP = socket (AF_INET, SOCK_DGRAM, 0);
	if (ls_UDP == -1) {
		perror(argv[0]);
		printf("%s: unable to create socket UDP\n", argv[0]);
		exit(1);
	}
	
	/* Bind the server's address to the socket. */
	if (bind(ls_UDP, (struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		printf("%s: unable to bind address UDP\n", argv[0]);
		exit(1);
	}

	/* Now, all the initialization of the server is
	 * complete, and any user errors will have already
	 * been detected.  Now we can fork the daemon and
	 * return to the user.  We need to do a setpgrp
	 * so that the daemon will no longer be associated
	 * with the user's control terminal.  This is done
	 * before the fork, so that the child will not be
	 * a process group leader.  Otherwise, if the child
	 * were to open a terminal, it would become associated
	 * with that terminal as its control terminal.  It is
	 * always best for the parent to do the setpgrp.
	 */
	setpgrp();

	switch (fork()) {
		case -1:	/* Unable to fork, for some reason. */
			perror(argv[0]);
			fprintf(stderr, "%s: unable to fork daemon\n", argv[0]);
			exit(1);

		
		case 0:		/* The child process (daemon) comes here. */

			/* Close stdin and stderr so that they will not
			 * be kept open.  Stdout is assumed to have been
			 * redirected to some logging file, or /dev/null.
			 * From now on, the daemon will not report any
			 * error messages.  This daemon will loop forever,
			 * waiting for connections and forking a child
			 * server to handle each one.
			 */
			fclose(stdin);
			fclose(stderr);

			/* Set SIGCLD to SIG_IGN, in order to prevent
			 * the accumulation of zombies as each child
			 * terminates.  This means the daemon does not
			 * have to make wait calls to clean them up.
			 */
			if ( sigaction(SIGCHLD, &sa, NULL) == -1) {
		    		perror(" sigaction(SIGCHLD)");
		    		fprintf(stderr,"%s: unable to register the SIGCHLD signal\n", argv[0]);
		    		exit(1);
		    	}
		    
			/* Register SIGTERM to create an orderly completion  */
			vec.sa_handler = (void *) endProgram;
			vec.sa_flags = 0;
			if ( sigaction(SIGTERM, &vec, (struct sigaction *) 0) == -1) {
		    		perror(" sigaction(SIGTERM)");
	    			fprintf(stderr,"%s: unable to register the SIGTERM signal\n", argv[0]);
		    		exit(1);
		    	}
		
			while (!END_LOOP) {
				/* Add both sockets to the mask */
				FD_ZERO(&readmask);
				FD_SET(ls_TCP, &readmask);
				FD_SET(ls_UDP, &readmask);
				
				/* Select the socket descriptor that has changed. It leaves a 
				   mark in the mask. */
				if (ls_TCP > ls_UDP) 	s_bigger=ls_TCP;
				else 		  	s_bigger=ls_UDP;

				if ( (numfds = select(s_bigger+1, &readmask, (fd_set *)0, (fd_set *)0, NULL)) < 0) {
					if (errno == EINTR) {
				    		END_LOOP=true;
				    		close(ls_TCP);
				    		close(ls_UDP);
				    		perror("\nFinalizando el servidor. Senial recibida en select\n"); 
					}
				}
				else { 
					/* Check if the selected socket is TCP */
					if (FD_ISSET(ls_TCP, &readmask)) {
						/* Note that addrlen is passed as a pointer
					     	 * so that the accept call can return the
					     	 * size of the returned address.
					     	 */
					     	 
						/* This call will block until a new
						 * connection arrives.  Then, it will
						 * return the address of the connecting
						 * peer, and a new socket descriptor, s,
						 * for that connection.
						 */
						s_TCP = accept(ls_TCP, (struct sockaddr *) &clientaddr_in, &addrlen);
						if (s_TCP == -1) 
							exit(1);
						
						switch (fork()) {
							case -1:	/* Can't fork, just exit. */
								exit(1);
							
							case 0:		/* Child process comes here. */
								/* Close the listen socket inherited from the daemon. */
								close(ls_TCP);
								serverTCP(s_TCP, clientaddr_in);
								exit(0);
							
							default:	/* Daemon process comes here. */
									/* The daemon needs to remember
									 * to close the new abrept socket
									 * after forking the child.  This
									 * prevents the daemon from running
									 * out of file descriptor space.  It
									 * also means that when the server
									 * closes the socket, that it will
									 * allow the socket to be destroyed
									 * since it will be the last close.
									 */
								close(s_TCP);
						}
						
					} /* End TCP*/
					
					
					
					/* Check if the selected socket is UDP */
					if (FD_ISSET(ls_UDP, &readmask)) {
						/* This call will block until a new
						* request arrives.  Then, it will create
						* a false "TCP" conexion and working the same
						* as TCP works creating a new socket for that
						* false conexion.
						*/
						br = recvfrom(ls_UDP, buffer, 1, 0, (struct sockaddr *)&clientaddr_in, &addrlen);
						if ( br == -1) {
						    perror(argv[0]);
						    printf("%s: recvfrom error (failed false conexion UDP)\n", argv[0]);
						    exit(1);
						}
						
						/* When a new client sends a UDP datagram, his information is stored
						* in "clientaddr_in", so we can create a false conexion by sending messages
						* manually with this information
						*/
						s_UDP = socket(AF_INET, SOCK_DGRAM, 0);
						if (s_UDP == -1) {
							perror(argv[0]);
							printf("%s: unable to create new socket UDP for new client\n", argv[0]);
							exit(1);
						}

						/* Clear and set up address structure for new socket. 
						* Port 0 is specified to get any of the avaible ones, as well as the IP address.
						*/						
						//memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
						myaddr_in.sin_family = AF_INET;
						myaddr_in.sin_addr.s_addr = INADDR_ANY;
						myaddr_in.sin_port = htons(0);
						
						/* Bind the server's address to the new socket for the client. */
						if (bind(s_UDP, (struct sockaddr *) &myaddr_in, sizeof(struct sockaddr_in)) == -1) {
							perror(argv[0]);
							printf("%s: unable to bind address new socket UDP for new client\n", argv[0]);
							exit(1);
						}
						
						/* As well as its done in TCP, a new thread is created for that false conexion */
						switch (fork()) {
							case -1:	
								exit(1);
								
							case 0:		/* Child process comes here. */
								/* Child doesnt need the listening socket */
					    			close(ls_UDP); 
					    			
					    			/* Sends a message to the client for him to know the new port for 
								* the false conexion
					    			*/
					    			if (sendto(s_UDP, " ", 1, 0, (struct sockaddr *)&clientaddr_in, addrlen) == -1) {
									perror(argv[0]);
									fprintf(stderr, "%s: unable to send request to \"connect\" \n", argv[0]);
									exit(1);
								}
								
																	
								serverUDP(s_UDP, clientaddr_in);
								exit(0);
							
							default:
								close(s_UDP);
						}
					
					} /* End UDP*/

				}

			}   /* End new clients loop */
			
			
			/* Close sockets before stopping the server */
			close(ls_TCP);
			close(ls_UDP);
		    
			printf("\nFin de programa servidor!\n");
		
		
		default:		/* Parent process comes here. */
			exit(0);
		}

	} //End switch	














/*
 *				S E R V E R T C P
 *
 *	This is the actual server routine that the daemon forks to
 *	handle each individual connection.  Its purpose is to receive
 *	the request packets from the remote client, process them,
 *	and return the results to the client.  It will also write some
 *	logging information to stdout.
 *
 */
void serverTCP(int s, struct sockaddr_in clientaddr_in)
{
	char hostname[MAXHOST];		/* remote host's name string */

	int len, len1, status;
	long timevar;			/* contains time returned by time() */
	struct linger linger;		/* allow a lingering, graceful close; */
				    	/* used when setting SO_LINGER */
			
	int i;	
	bool commandOK;
	char command[COMMAND_SIZE];
	
	
	/* Look up the host information for the remote host
	 * that we have connected with.  Its internet address
	 * was returned by the accept call, in the main
	 * daemon loop above.
	 */
	 
	status = getnameinfo((struct sockaddr *)&clientaddr_in,sizeof(clientaddr_in), hostname, MAXHOST,NULL,0,0);
	if(status){
		/* The information is unavailable for the remote
			 * host.  Just format its internet address to be
			 * printed out in the logging information.  The
			 * address will be shown in "internet dot format".
			 */
		 /* inet_ntop para interoperatividad con IPv6 */
		if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostname, MAXHOST) == NULL)
			perror(" inet_ntop \n");
	}
	
	/* Log a startup message. */
	time (&timevar);
		/* The port number must be converted first to host byte
		 * order before printing.  On most hosts, this is not
		 * necessary, but the ntohs() call is included here so
		 * that this program could easily be ported to a host
		 * that does require it.
		 */
	printf("[SERV TCP] Startup from %s port %u at %s",
		hostname, ntohs(clientaddr_in.sin_port), (char *) ctime(&timevar));

		/* Set the socket for a lingering, graceful close.
		 * This will cause a final close of this socket to wait until all of the
		 * data sent on it has been received by the remote host.
		 */
	linger.l_onoff  =1;
	linger.l_linger =1;
	if (setsockopt(s, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger)) == -1) {
		errout(hostname);
	}

		/* Go into a loop, receiving requests from the remote
		 * client.  After the client has sent the last request,
		 * it will do a shutdown for sending, which will cause
		 * an end-of-file condition to appear on this end of the
		 * connection.  After all of the client's requests have
		 * been received, the next recv call will return zero
		 * bytes, signalling an end-of-file condition.  This is
		 * how the server will know that no more requests will
		 * follow, and the loop will be exited.
		 */
		 
		 
	FILE* fd = fopen("../src/server/cTCP.txt", "w");
	
	while (len = recv(s, command, COMMAND_SIZE, 0)) {
		if (len == -1) errout(hostname); /* error from recv */
			/* The reason this while loop exists is that there
			 * is a remote possibility of the above recv returning
			 * less than COMMAND_SIZE bytes.  This is because a recv returns
			 * as soon as there is some data, and will not wait for
			 * all of the requested data to arrive.  Since COMMAND_SIZE bytes
			 * is relatively small compared to the allowed TCP
			 * packet sizes, a partial receive is unlikely.  If
			 * this example had used 2048 bytes requests instead,
			 * a partial receive would be far more likely.
			 * This loop will keep receiving until all COMMAND_SIZE bytes
			 * have been received, thus guaranteeing that the
			 * next recv at the top of the loop will start at
			 * the begining of the next request.
			 */
		while (len < COMMAND_SIZE) {
			len1 = recv(s, &command[len], COMMAND_SIZE-len, 0);
			if (len1 == -1) errout(hostname);
			len += len1;
		}


	
		/* Check if command has been received correctly */
		i=0;
		commandOK = false;
		while(i<COMMAND_SIZE){
			if(command[i] == '\r' && command[i+1] == '\n'){
				/* Command is correct because it founds "\r\n", so it just 
				* replaces that \r\n at the end of the command info by a "\0" 
				* just to work with it as a string
				*/
				command[i] = '\0';
				commandOK = true;
				break;
			}
			
			if(i == COMMAND_SIZE-2){
				/* Command is wrong because it doesnt finish with "\r\n"
				*/
				commandOK = false;
				fprintf(stderr, "Error, command received incorrectly, no \\r\\n \n");
				errout(hostname);
				break;
			}
			i++;
		}
		
		/* Command is wrong, sends an error message and continues (should stop)*/
		if(!commandOK){
			//TODO: command is wrong, send message 
			fprintf(stderr, "mal\n");
			continue;
		}
		
		
		
//--------	/* Command is ok, just works :D */
		switch(checkCommand(command)){
			case LIST:
				fprintf(fd, "%-16s -> %s\n","Comand LIST:" , command);
				break;
			
			case NEWGROUPS:
				fprintf(fd, "%-16s -> %s\n","Comand NEWGROUPS:" , command);
				break;
			
			case NEWNEWS:
				fprintf(fd, "%-16s -> %s\n","Comand NEWNEWS:" , command);
				break;
				
			case GROUP:
				fprintf(fd, "%-16s -> %s\n","Comand GROUP:" , command);
				break;
			
			case ARTICLE:
				fprintf(fd, "%-16s -> %s\n","Comand ARTICLE:" , command);
				break;
				
			case HEAD:
				fprintf(fd, "%-16s -> %s\n","Comand HEAD:" , command);
				break;
			
			case BODY:
				fprintf(fd, "%-16s -> %s\n","Comand BODY:" , command);
				break;
			
			case POST:
				fprintf(fd, "%-16s -> %s\n","Comand POST:" , command);
				break;
							
			case QUIT:
				fprintf(fd, "%-16s -> %s\n","Comand QUIT:" , command);
				break;
				
				
			default:
				fprintf(fd, "%-16s -> %s\n","Wrong command D:" , command);
		}
		


		strtok(command," ");
		/* Send a response back to the client. */
		if (send(s, command, COMMAND_SIZE, 0) != COMMAND_SIZE) 
			errout(hostname);
	}
	
	fclose(fd);

		/* The loop has terminated, because there are no
		 * more requests to be serviced.  As mentioned above,
		 * this close will block until all of the sent replies
		 * have been received by the remote host.  The reason
		 * for lingering on the close is so that the server will
		 * have a better idea of when the remote has picked up
		 * all of the data.  This will allow the start and finish
		 * times printed in the log file to reflect more accurately
		 * the length of time this connection was used.
		 */
	close(s);

		/* Log a finishing message. */
	time (&timevar);
		/* The port number must be converted first to host byte
		 * order before printing.  On most hosts, this is not
		 * necessary, but the ntohs() call is included here so
		 * that this program could easily be ported to a host
		 * that does require it.
		 */
	printf("[SERV TCP] Completed %s port %u  at %s",
		hostname, ntohs(clientaddr_in.sin_port), (char *) ctime(&timevar));
}





/*
 *	This routine aborts the child process attending the client.
 */
void errout(char *hostname)
{
	printf("Connection with %s aborted on error\n", hostname);
	exit(1);     
}
















/*
 *				S E R V E R U D P
 *
 *	This is the actual server routine that the daemon forks to
 *	handle each individual connection.  Its purpose is to receive
 *	the request packets from the remote client, process them,
 *	and return the results to the client.  It will also write some
 *	logging information to stdout.
 *
 */
void serverUDP(int s, struct sockaddr_in clientaddr_in)
{
	char hostname[MAXHOST];		/* remote host's name string */

	int status, n_retry;
	long timevar;			/* contains time returned by time() */
	int addrlen = sizeof(struct sockaddr_in);
	
	bool finish = false;
	
	int i;	
	bool commandOK;
	char command[COMMAND_SIZE];

				
	/* Look up the host information for the remote host
	 * that we have connected with.  Its internet address
	 * was returned by the accept call, in the main
	 * daemon loop above.
	 */
	 
	status = getnameinfo((struct sockaddr *)&clientaddr_in,sizeof(clientaddr_in), hostname, MAXHOST,NULL,0,0);
	if(status){
		/* The information is unavailable for the remote
			 * host.  Just format its internet address to be
			 * printed out in the logging information.  The
			 * address will be shown in "internet dot format".
			 */
		 /* inet_ntop para interoperatividad con IPv6 */
		if (inet_ntop(AF_INET, &(clientaddr_in.sin_addr), hostname, MAXHOST) == NULL)
			perror(" inet_ntop \n");
	}
	
	time (&timevar);
	printf("[SERV UDP] Startup from %s port %u at %s",
		hostname, ntohs(clientaddr_in.sin_port), (char *) ctime(&timevar));

	
	
	FILE* fd = fopen("../src/server/cUDP.txt", "w");
	
	while (1) {
		//Receives next command from the client
		n_retry = RETRIES;
		while(n_retry > 0){
			alarm(TIMEOUT);
			if (recvfrom(s, command, COMMAND_SIZE, 0, (struct sockaddr *)&clientaddr_in, &addrlen) == -1) {
				if (errno == EINTR) {
		 		     fprintf(stderr,"[SERV UDP] Alarm went off.\n");
		 		     n_retry--; 
				} else  {
					fprintf(stderr,"[SERV UDP] Unable to get response to \"connect\"\n");
					exit(1); 
				}
			}else{
				alarm(0);
				break;
			}
		}
		
		if(n_retry == 0){ 
			break;
		}
	
		/* Check if command has been received correctly */
		i=0;
		commandOK = false;
		while(i<COMMAND_SIZE){
			if(command[i] == '\r' && command[i+1] == '\n'){
				/* Command is correct because it founds "\r\n", so it just 
				* replaces that \r\n at the end of the command info by a "\0" 
				* just to work with it as a string
				*/
				command[i] = '\0';
				commandOK = true;
				break;
			}
			
			if(i == COMMAND_SIZE-2){
				/* Command is wrong because it doesnt finish with "\r\n"
				*/
				commandOK = false;
				fprintf(stderr, "Error, command received incorrectly, no \\r\\n \n");
				errout(hostname);
				break;
			}
			i++;
		}
		
		/* Command is wrong, sends an error message and continues (should stop)*/
		if(!commandOK){
			//TODO: command is wrong, send message 
			fprintf(stderr, "mal\n");
			continue;
		}
		
		
		
//--------	/* Command is ok, just works :D */
		switch(checkCommand(command)){
			case LIST:
				fprintf(fd, "%-16s -> %s\n","Comand LIST:" , command);
				break;
			
			case NEWGROUPS:
				fprintf(fd, "%-16s -> %s\n","Comand NEWGROUPS:" , command);
				break;
			
			case NEWNEWS:
				fprintf(fd, "%-16s -> %s\n","Comand NEWNEWS:" , command);
				break;
				
			case GROUP:
				fprintf(fd, "%-16s -> %s\n","Comand GROUP:" , command);
				break;
			
			case ARTICLE:
				fprintf(fd, "%-16s -> %s\n","Comand ARTICLE:" , command);
				break;
				
			case HEAD:
				fprintf(fd, "%-16s -> %s\n","Comand HEAD:" , command);
				break;
			
			case BODY:
				fprintf(fd, "%-16s -> %s\n","Comand BODY:" , command);
				break;
			
			case POST:
				fprintf(fd, "%-16s -> %s\n","Comand POST:" , command);
				break;
							
			case QUIT:
				fprintf(fd, "%-16s -> %s\n","Comand QUIT:" , command);
				finish = true;
				break;
				
				
			default:
				fprintf(fd, "%-16s -> %s\n","Wrong command D:" , command);
		}
		

		strtok(command," ");
		/* Send a response back to the client. */
		if (sendto(s, command, COMMAND_SIZE, 0, (struct sockaddr *)&clientaddr_in, addrlen) == -1) 
			errout(hostname);
	
		if(strcmp(command, "QUIT") == 0 || finish)
			break;
	}
	
	fclose(fd);		

	close(s);
	
	time (&timevar);
	printf("[SERV UDP] Completed %s port %u  at %s",
		hostname, ntohs(clientaddr_in.sin_port), (char *) ctime(&timevar));
 }
 
 
 
 
 
 
 
 
 
/* Checks what command is */
int checkCommand(char *command){
	int i;
	int maxLengthCommand;
	GET_LONGEST_COMMAND(NCOMMANDS, maxLengthCommand)
	char realCommand[maxLengthCommand];
	
	strcpy(realCommand, strtok( strdup(command), " "));	
	for (i=0; i < NCOMMANDS; i++) {
		Command *com = &commandTable[i];
		if (strcmp(com->command, realCommand) == 0)
		    return com->id;
	}
	
	return WRONG_COMMAND;
}
