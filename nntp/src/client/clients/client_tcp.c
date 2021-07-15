#include "../client_commands.h"
#include "client_tcp.h"



int clienttcp(char** argv)
{
	int s;				/* connected socket descriptor */
	struct addrinfo hints, *res;
	long timevar;			/* contains time returned by time() */
	struct sockaddr_in myaddr_in;	/* for local socket address */
	struct sockaddr_in servaddr_in;	/* for server socket address */
	int addrlen, i, j, errcode;
	
	char command[COMMAND_SIZE];	/* This example uses COMMAND_SIZE byte messages. */
	char response[COMMAND_SIZE];   
	FILE *commandsFile;		/* File that contains client NNTP commands to be executed */
	
	/* Create the socket. */
	s = socket (AF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to create socket\n", argv[0]);
		exit(1);
	}
	
	/* clear out address structures */
	memset ((char *)&myaddr_in, 0, sizeof(struct sockaddr_in));
	memset ((char *)&servaddr_in, 0, sizeof(struct sockaddr_in));

	/* Set up the peer address to which we will connect. */
	servaddr_in.sin_family = AF_INET;
	
	/* Get the host information for the hostname that the
	 * user passed in. */
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

		/* Try to connect to the remote server at the address
		 * which was just built into peeraddr.
		 */
	if (connect(s, (const struct sockaddr *)&servaddr_in, sizeof(struct sockaddr_in)) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to connect to remote\n", argv[0]);
		exit(1);
	}
		/* Since the connect call assigns a free address
		 * to the local end of this connection, let's use
		 * getsockname to see what it assigned.  Note that
		 * addrlen needs to be passed in as a pointer,
		 * because getsockname returns the actual length
		 * of the address.
		 */
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
	printf("[TCP] Connected to %s on port %u at %s",
			argv[1], ntohs(myaddr_in.sin_port), (char *) ctime(&timevar));


//---------

	commandsFile = fopen("../src/client/someNNTPCommands.txt", "r");
	if(commandsFile == NULL){
		fprintf(stderr, "[TCP] Cannot read NNTP commands file\n");
		exit(1);
	}
	
	RESET(command, COMMAND_SIZE);
	while( fgets(command, sizeof(command), commandsFile) != NULL){
	
		command[strlen(command) - 2] = '\0';
		addCRLF(command, COMMAND_SIZE);
		
		if (send(s, command, COMMAND_SIZE, 0) != COMMAND_SIZE) {
			fprintf(stderr, "%s: Connection aborted on error ", argv[0]);
			fprintf(stderr, "on send number %d\n", i);
			exit(1);
		}
		
		if(removeCRLF(command)){
			fprintf(stderr, "[TCP] Command without CR-LF. Aborted \"conexion\" \n");
			exit(1);
		}
		
		RESET(response, COMMAND_SIZE);
		
		printf("\nC:\"%s\"\n", command);
		
		switch(checkCommand(command)){
			case LIST:
				//Received response
				i = recv(s, response, COMMAND_SIZE, 0);
				if (i == -1) {
			    		perror(argv[0]);
					fprintf(stderr, "[TCP] %s: error reading result\n", argv[0]);
					exit(1);
				}
				while (i < COMMAND_SIZE) {
					j = recv(s, &response[i], COMMAND_SIZE-i, 0);
					if (j == -1) {
				     		perror(argv[0]);
						fprintf(stderr, "[TCP] %s: error reading result\n", argv[0]);
						exit(1);
			       		}
			       		
					i += j;
				}
				
				
				//Change CRLF to '\0' to work with response as a string
				if(removeCRLF(response)){
					fprintf(stderr, "[TCP] Response without CR-LF. Aborted conexion\n");
					exit(1);
				}
				
				printf("S: %s\n", response);
				
				
				//Check response code
				if(RESP_200(GET_CODE(response))){
					while(1){
						i = recv(s, response, COMMAND_SIZE, 0);
						if (i == -1) {
					    		perror(argv[0]);
							fprintf(stderr, "[TCP] %s: error reading result\n", argv[0]);
							exit(1);
						}
						while (i < COMMAND_SIZE) {
							j = recv(s, &response[i], COMMAND_SIZE-i, 0);
							if (j == -1) {
						     		perror(argv[0]);
								fprintf(stderr, "[TCP] %s: error reading result\n", argv[0]);
								exit(1);
					       		}
					       		
							i += j;
						}
						
						
						if(removeCRLF(response)){
							fprintf(stderr, "[TCP] Response without CR-LF. Aborted conexion\n");
							exit(1);
						}
						
						if(FINISH_RESP(response)) break;
						
						printf("S: %s\n", response);
					}
				}
				break;
			
			case NEWGROUPS:
				break;
			
			case NEWNEWS:
				break;
				
			case GROUP:
				break;
			
			case ARTICLE:
				break;
				
			case HEAD:
				break;
			
			case BODY:
				break;
			
			case POST:
				break;
							
			case QUIT:
				printf("S:%s\n", "BYE :D");
				break;
				
			default:
				printf("S:%s\n", "Wrong command :D");
			
		}
		
		RESET(command, COMMAND_SIZE);
	}

	fclose(commandsFile);



//---------
	/* Now, shutdown the connection for further sends.
	 * This will cause the server to receive an end-of-file
	 * condition after it has received all the requests that
	 * have just been sent, indicating that we will not be
	 * sending any further requests.
	 */
	if (shutdown(s, 1) == -1) {
		perror(argv[0]);
		fprintf(stderr, "%s: unable to shutdown socket\n", argv[0]);
		exit(1);
	}



    /* Print message indicating completion of task. */
	time(&timevar);
	printf("\n[TCP] All done at %s", (char *)ctime(&timevar));
}


