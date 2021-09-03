#ifndef __CLIENT_COMMANDS_H__
#define __CLIENT_COMMANDS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "params.h"

#define RESET(s, size) (memset(s, '\0', size))

#define FINISH_RESP(resp) (resp[0] == '.')

#define GET_CODE(resp) (atoi(strtok( strdup(resp), " ")))

#define RESP_200(code) (code >= 200 && code < 300)
#define RESP_300(code) (code >= 300 && code < 400)
#define RESP_400(code) (code >= 400 && code < 500)
#define RESP_500(code) (code >= 500 && code < 600)

typedef struct commandResponse{
	unsigned short int code;
	char message[COMMAND_SIZE];
} CommandResponse;


//Other functions
int numLines(FILE *file);
void addCRLF(char *s, int size);
int removeCRLF(char *s);
int checkCommand(char *command);


#endif
