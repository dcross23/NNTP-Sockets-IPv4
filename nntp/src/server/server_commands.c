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
	
	comResp = (CommandResponse){215, "215 Listado de los grupos en formato \"nombre ultimo primero fecha descripcion\"."};
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



/**
 * NEWGROUPS command
 */
CommandResponse newgroups(char *command, char ***groupsMatched, int *nGroups){
	FILE *groupsFile;
	CommandResponse comResp;
	
	char *date, *hour, *gdate, *ghour;
	regex_t dateHourRegex;
	bool sintaxError = false;

	uint16_t day, month, year, hours, min, sec;
	uint16_t gday, gmonth, gyear, ghours, gmin, gsec;
	int i;

	char group[COMMAND_SIZE];
	char *gName;


	if(!regcomp(&dateHourRegex, DATE_HOUR_REGEX, REG_EXTENDED)){
		perror("DateHourRegex \n");
	}

	strtok(command, " "); //Discards 'NEWSGROUPS' (name of the command)
	
	//Invalid date
	date = strtok(NULL, " ");
	if(regexec(&dateHourRegex, date, 0, NULL, 0) == REG_NOMATCH) sintaxError = true;

	//Invalid hour
	hour = strtok(NULL, " ");
	if(regexec(&dateHourRegex, hour, 0, NULL, 0) == REG_NOMATCH) sintaxError = true;

	//Invalid number of arguments
	if(strtok(NULL, " ") != NULL) sintaxError = true;

	if(sintaxError){
		comResp = (CommandResponse){501, "501 Error de sintaxis en NEWGROUPS YYMMDD HHMMSS."};
		addCRLF(comResp.message, COMMAND_SIZE);
		return comResp;
	}


	//Sintax is correct, now parse command
	if(NULL == (groupsFile = fopen("../noticias/grupos", "r"))){
		comResp = (CommandResponse){-1, "Error"};
		return comResp;
	}

	year = atoi(date)/10000;
	month = (atoi(date) % 10000) / 100;
	day = atoi(date) % 100;
	
	hours = atoi(hour)/10000;
	min = (atoi(hour) % 10000) / 100;
	sec = atoi(hour) % 100;
    	
	comResp = (CommandResponse){231, ""};
	sprintf(comResp.message, "231 Nuevos grupos desde %02d/%02d/%02d %02d:%02d:%02d.",day, month, year, hours, min, sec);
	addCRLF(comResp.message, COMMAND_SIZE);

	i=0;
	fseek(groupsFile, 0, SEEK_SET);
	while( fgets(group, COMMAND_SIZE, groupsFile) ){
		group[strlen(group) - 1] = '\0';
		
		gName = strtok(group, " ");
		strtok(NULL, " "); //Discard group number
		strtok(NULL, " "); //Discard group number

		gdate = strtok(NULL, " ");;
		ghour = strtok(NULL, " ");

		gyear = atoi(gdate)/10000;
		gmonth = (atoi(gdate) % 10000) / 100;
		gday = atoi(gdate) % 100;
	
		ghours = atoi(ghour)/10000;
		gmin = (atoi(ghour) % 10000) / 100;
		gsec = atoi(ghour) % 100;


		//Creation date is greater or creation date is the same but hour is greatter
		if(DATE(gyear,gmonth,gday)>DATE(year,month,day) || 
		   (DATE(gyear,gmonth,gday)==DATE(year,month,day) && HOUR(ghours,gmin,gsec)>HOUR(hours,min,sec)) ){

			REALLOC_SV( (*groupsMatched), *nGroups, (*nGroups + 1) )
			(*nGroups)++;

			(*groupsMatched)[i] = malloc(COMMAND_SIZE * sizeof(char));
			strcpy( (*groupsMatched)[i] , gName);
			addCRLF((*groupsMatched)[i], COMMAND_SIZE);
			i++;

		}				
	}

	//Add . as last groups to finish emision
	REALLOC_SV( (*groupsMatched), *nGroups, (*nGroups + 1) )
	(*nGroups)++;
	(*groupsMatched)[i] = malloc(COMMAND_SIZE * sizeof(char));
	strcpy( (*groupsMatched)[i] , ".");
	addCRLF((*groupsMatched)[i], COMMAND_SIZE);


	fclose(groupsFile);
	return comResp;	
}



/**
 * NEWNEWS command
 */
CommandResponse newnews(char *command){
	CommandResponse comResp;
	FILE *groupsFile;
	DIR *groupDir;
	char groupRoute[COMMAND_SIZE], aux[COMMAND_SIZE];
	char *subgroup;
	
	char *date, *hour, *group, *gdate, *ghour;
	regex_t dateHourRegex, groupRegex;
	bool sintaxError = false;

	uint16_t day, month, year, hours, min, sec;
	uint16_t gday, gmonth, gyear, ghours, gmin, gsec;
	int i;

	char *gName;

	if(!regcomp(&dateHourRegex, DATE_HOUR_REGEX, REG_EXTENDED)){
		perror("DateHourRegex \n");
	}

	if(!regcomp(&groupRegex, NEWNEWS_GROUP_REGEX, REG_EXTENDED)){
		perror("GroupRegex \n");
	}


	strtok(command, " "); //Discards 'NEWNEWS' (name of the command)
	
	//Invalid group
	group = strtok(NULL, " ");
	if(regexec(&groupRegex, group, 0, NULL, 0) == REG_NOMATCH) sintaxError = true;

	if(sintaxError){
		printf("\n F por el regex del grupo \n");
	}

	//Invalid date
	date = strtok(NULL, " ");
	if(regexec(&dateHourRegex, date, 0, NULL, 0) == REG_NOMATCH) sintaxError = true;

	//Invalid hour
	hour = strtok(NULL, " ");
	if(regexec(&dateHourRegex, hour, 0, NULL, 0) == REG_NOMATCH) sintaxError = true;

	//Invalid number of arguments
	if(strtok(NULL, " ") != NULL) sintaxError = true;

	if(sintaxError){
		comResp = (CommandResponse){501, "501 Error de sintaxis en NEWNEWS GROUP YYMMDD HHMMSS."};
		addCRLF(comResp.message, COMMAND_SIZE);
		return comResp;
	}


	//Sintax is correct, now parse command
	//Get date and hour
	year = atoi(date)/10000;
	month = (atoi(date) % 10000) / 100;
	day = atoi(date) % 100;
	
	hours = atoi(hour)/10000;
	min = (atoi(hour) % 10000) / 100;
	sec = atoi(hour) % 100;


	//Get full route to get the group
	strcpy(groupRoute, "../noticias/articulos");
	strcpy(aux, "/"); strcat(aux, strtok(group,"."));
	strcat(groupRoute, aux);

	while(NULL != (subgroup = strtok(NULL,"."))){
		strcat(groupRoute, "/");
		strcat(groupRoute, subgroup);
	}

	groupDir = opendir(groupRoute);
	//Directory exists
	if(groupDir){
		closedir(groupDir);

		if(NULL == (groupsFile = fopen("../noticias/grupos", "r"))){
			comResp = (CommandResponse){-1, "Error"};
			addCRLF(comResp.message, COMMAND_SIZE);
			return comResp;
		}

		while( fgets(aux, COMMAND_SIZE, groupsFile) ){
			aux[strlen(aux) - 1] = '\0';
			
			
					
		}


		comResp = (CommandResponse){230, ""};
		sprintf(comResp.message, "230 Nuevos articulos desde %02d/%02d/%02d %02d:%02d:%02d.",day, month, year, hours, min, sec);
		addCRLF(comResp.message, COMMAND_SIZE);
		return comResp;
	}
	//Directory does not exist
	else{
		comResp = (CommandResponse){411, "411 No existe ese grupo de noticias."};
		addCRLF(comResp.message, COMMAND_SIZE);
	}

	return comResp;
} 







