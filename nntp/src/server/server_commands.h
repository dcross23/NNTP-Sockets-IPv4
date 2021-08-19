#ifndef __SERVER_COMMANDS_H__
#define __SERVER_COMMANDS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <dirent.h>

#include "../params.h"

//REGEX
static const char DATE_HOUR_REGEX[] = "^[0-9]{6}$";
static const char GROUP_REGEX[] = "^[^\\.](.+)\\.((.+)\\.)*(.*)[^\\.]$";
static const char ARTICLE_REGEX[] = "^[0-9]+$";


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


/**
 * NEWNEWS:
 * @param group
 * @param YYMMDD
 * @param HHMMSS
 * Sends to the client the number, id and subject of the article in the group specified
 *  that was created at a later date and time that the ones specified in params with format 
 *  YYMMDD (year, month and day) and HHMMSS (hour, minute, second) 
 */
CommandResponse newnews(char *command, char ***articlesMatched, int *nArticles);



/**
 * GROUP:
 * @param group
 * Selects the group specified for ARTICLE, HEAD and BODY commands and sends to the client
 *  the number of articles and the number of first and last articles.
 */
CommandResponse group(char *command, bool *isGroupSelected, char *groupSelected);



/**
 * ARTICLE:
 * @param article
 * Allows the client to check an specific article.
 */
CommandResponse article(char *command, bool isGroupSelected, char *groupSelected, char ***articleInfo, int *nLines);

#endif
