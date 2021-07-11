#ifndef __SERVER_COMMANDS_H__
#define __SERVER_COMMANDS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../params.h"


typedef struct commandResponse{
	unsigned short int code;
	char message[COMMAND_SIZE];
} CommandResponse;


//Other functions
int numLines(FILE *file);
void addCRLF(char *s, int size);
int removeCRLF(char *s);


/**
 * LIST:
 *	Sends to the client the name, number of last article, number of first 
 *	 article, date and description of the different groups.   
 */
CommandResponse list(char ***groupsInfo, int *nGroups);


#endif
