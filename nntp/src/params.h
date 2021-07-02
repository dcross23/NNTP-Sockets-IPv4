#ifndef __PARAMS_H__
#define __PARAMS_H__

#define PORT 		7692   		// > Servers listening port
#define ADDRNOTFOUND 	0xffffffff	// > Addres for unfound host
#define COMMAND_SIZE	512		// > Max size for packets received

//Server and clientUDP
#define RETRIES	5			// > Number of times to retry before givin up
#define TIMEOUT 6			// > Max time for getting a response


//Commands
typedef struct {
	char *command;
	int id;
} Command;

enum commandEnum {LIST, NEWGROUPS, NEWNEWS, GROUP, ARTICLE, HEAD, BODY, POST, QUIT, ENUM_TAM}; 

static Command commandTable[] = {
	{"LIST", LIST},
	{"NEWGROUPS", NEWGROUPS},
	{"NEWNEWS", NEWNEWS},
	{"GROUP", GROUP},
	{"ARTICLE", ARTICLE},
	{"HEAD", HEAD},
	{"BODY", BODY},
	{"POST", POST},
	{"QUIT", QUIT}
};

#define NCOMMANDS (sizeof(commandTable)/sizeof(Command))
#define WRONG_COMMAND -1

#define GET_LONGEST_COMMAND(numCommands, result) \
	do{	\
		int max = 0; 	\
		for(int counter=0; counter<numCommands; counter++){	\
			if(strlen(commandTable[counter].command) > max)		\
				max = strlen(commandTable[counter].command);	\
		}	\
		result = max;	\
	}while(0);
	
	

#endif
