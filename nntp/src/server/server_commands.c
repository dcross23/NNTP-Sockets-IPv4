#include "server_commands.h"


int numLines(FILE *file){
	char c;
	int lines = 0;
	
	while ((c = fgetc(file)) != EOF) 
        	if (c == '\n') lines++;   
        
        return lines;
}


void addCRLF(char *s, int size){
	int length;

	length = strlen(s);
	if(length >= size-2){
		s[size-2] = CR;
		s[size-1] = LF;
	}else{
		s[length] = CR;
		s[length+1] = LF;
	}
}

int removeCRLF(char *s){
	int i;

	i = 0;
	while(1){
		if(s[i] == CR && s[i+1] == LF){
			s[i] = '\0';
			return 0;
		}
		if (i == COMMAND_SIZE - 2){
			return 1;
		}
		i++;
	}
}


/**
 * LIST command
 */
CommandResponse list(char ***groupsInfo, int *nGroups){
	FILE *groupsFile;
	CommandResponse comResp;
	int i;
	char group[COMMAND_SIZE];
	
	if(NULL == (groupsFile = fopen("../noticias/grupos", "r"))){
		comResp = (CommandResponse){-1, "Error"};
		return comResp;
	}
	
	memset(comResp.message, '\0', COMMAND_SIZE);
	strcpy(comResp.message, "215 Listado de los grupos en formato \"nombre ultimo primero fecha descripcion\".");
	comResp.code = 215;
	
	addCRLF(comResp.message, COMMAND_SIZE);
	
	*nGroups = numLines(groupsFile) + 1;
	*groupsInfo = malloc( (*nGroups) * sizeof(char *));
	
	i=0;
	fseek(groupsFile, 0, SEEK_SET);
	while( fgets(group, COMMAND_SIZE, groupsFile) ){
		group[strlen(group) - 1] = '\0';
		
		(*groupsInfo)[i] = malloc(COMMAND_SIZE * sizeof(char));
		strcpy( (*groupsInfo)[i] , group);
		addCRLF((*groupsInfo)[i], COMMAND_SIZE);
		
		i++;				
	}
	
	//Send "." to finish sending commands
	(*groupsInfo)[i] = malloc(COMMAND_SIZE * sizeof(char));
	strcpy( (*groupsInfo)[i] , ".");
	addCRLF((*groupsInfo)[i], COMMAND_SIZE);
	
	
	fclose(groupsFile);
	
	return comResp;
}

