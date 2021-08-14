#ifndef __SERVER_COMMANDS_H__
#define __SERVER_COMMANDS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>

#include "../params.h"

//REGEX
static const char NEWSGROUPS_REGEX[] = "^[0-9]{6}$";


/**
 * Structure that stores info of the command response
 */
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

/**
 * NEWGROUPS:
 * @param YYMMDD 
 * @param HHMMSS
 *  Sends to the client the name of the group that was created at a later
 *   date and time that the ones specified in params with format YYMMDD (year,
 *   month and day) and HHMMSS (hour, minute, second) 
 */
#define DATE(y,m,d) (y*365+m*30+d)
#define HOUR(h,m,s) (h*3600+m*60+s)

//Creates or expands a char* (string) array
#define REALLOC_SV(v,actSize,newSize)  \
	do{	\
		if(actSize == 0) v = malloc( 1 * sizeof(char *)); \
		else v = realloc(v, (newSize) * sizeof(char *));	\
	}while(0);

CommandResponse newgroups(char *command, char ***groupsMatched, int *nGroups);


#endif