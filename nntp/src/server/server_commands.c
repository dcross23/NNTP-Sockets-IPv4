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


	if(regcomp(&dateHourRegex, DATE_HOUR_REGEX, REG_EXTENDED)){
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
CommandResponse newnews(char *command, char ***articlesMatched, int *nArticles){
	CommandResponse comResp;
	FILE *groupsFile, *articleFile;
	DIR *groupDir;
	char groupRoute[COMMAND_SIZE], articleRoute[COMMAND_SIZE];
	char aux[COMMAND_SIZE], *aux2;
	
	char *date, *hour, *group, *subgroup, *adate, *ahour;
	regex_t dateHourRegex, groupRegex;
	bool sintaxError = false;
	uint16_t day, month, year, hours, min, sec;
	uint16_t aday, amonth, ayear, ahours, amin, asec;

	int nLastArticle;
	char *rowName;
	char subject[COMMAND_SIZE];
	char id[COMMAND_SIZE];


	if(regcomp(&dateHourRegex, DATE_HOUR_REGEX, REG_EXTENDED)){
		perror("DateHourRegex \n");
	}

	if(regcomp(&groupRegex, GROUP_REGEX, REG_EXTENDED)){
		perror("GroupRegex \n");
	}


	strtok(command, " "); //Discards 'NEWNEWS' (name of the command)
	
	//Invalid group
	group = strtok(NULL, " ");
	if(regexec(&groupRegex, group, 0, NULL, 0) == REG_NOMATCH) sintaxError = true;

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
	strcpy(aux, "/"); strcat(aux, strtok(strdup(group), "."));
	strcat(groupRoute, aux);

	while(NULL != (subgroup = strtok(NULL,"."))){
		strcat(groupRoute, "/");
		strcat(groupRoute, subgroup);
	}

	groupDir = opendir(groupRoute);
	//Directory exists
	if(groupDir){
		closedir(groupDir);

		//Search the group specified in the groups file and get the number of the last article in the group
		if(NULL == (groupsFile = fopen("../noticias/grupos", "r"))){
			comResp = (CommandResponse){-1, "Error"};
			addCRLF(comResp.message, COMMAND_SIZE);
			return comResp;
		}

		while( fgets(aux, COMMAND_SIZE, groupsFile) ){
			if(strcmp(group, strtok(aux, " ")) == 0){
				nLastArticle = atoi(strtok(NULL, " "));
				break;
			}				
		}

		fclose(groupsFile);


		//For each article, get the ones that are newer than the date specified
		int j = 0;
		for(int i = 1; i<= nLastArticle; i++){
			strcpy(articleRoute, groupRoute);
			sprintf(aux, "/%d", i);
			strcat(articleRoute, aux);
			
			//Open the article and find the date
			if(NULL == (articleFile = fopen(articleRoute, "r"))){
				comResp = (CommandResponse){-1, "Error"};
				addCRLF(comResp.message, COMMAND_SIZE);
				return comResp;
			}

			while(fgets(aux, COMMAND_SIZE, articleFile)){
				removeCRLF(aux);
				rowName = strtok(aux, ":");

				if(strcmp("Subject", rowName) == 0){
					strcpy(subject, strtok(NULL, ":") + 1); //+1 to remove first space								
				}
				else if(strcmp("Date", rowName) == 0){
					aux2 = strtok(NULL, ":");
					adate = strtok(aux2, " ");
					ahour = strtok(NULL, " ");

					ayear = atoi(adate)/10000;
					amonth = (atoi(adate) % 10000) / 100;
					aday = atoi(adate) % 100;
				
					ahours = atoi(ahour)/10000;
					amin = (atoi(ahour) % 10000) / 100;
					asec = atoi(ahour) % 100;	
				}
				else if(strcmp("Message-ID", rowName) == 0){
					strcpy(id, strtok(NULL, ":") + 1); //+1 to remove first space				
					break;	
				}
			}

			sprintf(aux, "%d - %s - %s", i, id, subject);

			//Creation date is greater or creation date is the same but hour is greatter
			if(DATE(ayear,amonth,aday)>DATE(year,month,day) || 
			  (DATE(ayear,amonth,aday)==DATE(year,month,day) && HOUR(ahours,amin,asec)>HOUR(hours,min,sec)) ){

				REALLOC_SV( (*articlesMatched), (*nArticles), (*nArticles + 1) )
				(*nArticles)++;

				(*articlesMatched)[j] = malloc(COMMAND_SIZE * sizeof(char));
				
				strcpy( (*articlesMatched)[j] , aux);
				addCRLF((*articlesMatched)[j], COMMAND_SIZE);

				j++;
			}				

			fclose(articleFile);
		}

		//Add . to finish emision
		REALLOC_SV( (*articlesMatched), (*nArticles), (*nArticles + 1) )
		(*nArticles)++;
		(*articlesMatched)[j] = malloc(COMMAND_SIZE * sizeof(char));
		strcpy( (*articlesMatched)[j] , ".");
		addCRLF((*articlesMatched)[j], COMMAND_SIZE);

		comResp = (CommandResponse){230, ""};
		sprintf(comResp.message, "230 Nuevos articulos desde %02d/%02d/%02d %02d:%02d:%02d.",day, month, year, hours, min, sec);
		addCRLF(comResp.message, COMMAND_SIZE);
	}
	//Directory does not exist
	else{
		comResp = (CommandResponse){411, "411 No existe ese grupo de noticias."};
		addCRLF(comResp.message, COMMAND_SIZE);
	}


	return comResp;
} 



/**
 * GROUP command
 */
CommandResponse group(char *command, bool *isGroupSelected, char *groupSelected){
	CommandResponse comResp;
	FILE *groupsFile;
	DIR *groupDir;
	char groupRoute[COMMAND_SIZE];
	char aux[COMMAND_SIZE];

	char *group, *subgroup;
	int firstArticle, lastArticle;
	bool sintaxError = false;
	regex_t groupRegex;


	if(regcomp(&groupRegex, GROUP_REGEX, REG_EXTENDED)){
		perror("GroupRegex \n");
	}	

	strtok(command, " "); //Discards 'GROUP' (name of the command)
	
	//Invalid group
	group = strtok(NULL, " ");
	if(regexec(&groupRegex, group, 0, NULL, 0) == REG_NOMATCH) sintaxError = true;

	//Invalid number of arguments
	if(strtok(NULL, " ") != NULL) sintaxError = true;

	if(sintaxError){
		comResp = (CommandResponse){501, "501 Error de sintaxis en GROUP newsgroup."};
		addCRLF(comResp.message, COMMAND_SIZE);
		*isGroupSelected = false;
		strcpy(groupSelected, group);
		return comResp;
	}


	//Sintax is correct, now parse command
	//Get full route to get the group
	strcpy(groupRoute, "../noticias/articulos");
	strcpy(aux, "/"); strcat(aux, strtok(strdup(group), "."));
	strcat(groupRoute, aux);

	while(NULL != (subgroup = strtok(NULL,"."))){
		strcat(groupRoute, "/");
		strcat(groupRoute, subgroup);
	}

	//Check if groups exists
	groupDir = opendir(groupRoute);
	//Directory exists
	if(groupDir){
		closedir(groupDir);

		//Search the group specified in the groups file
		if(NULL == (groupsFile = fopen("../noticias/grupos", "r"))){
			comResp = (CommandResponse){-1, "Error"};
			addCRLF(comResp.message, COMMAND_SIZE);
			return comResp;
		}

		while( fgets(aux, COMMAND_SIZE, groupsFile) ){
			if(strcmp(group, strtok(aux, " ")) == 0){
				lastArticle = atoi(strtok(NULL, " "));
				firstArticle = atoi(strtok(NULL, " "));
				break;
			}				
		}

		fclose(groupsFile);

		comResp = (CommandResponse){211, ""};
		if(lastArticle < firstArticle)
			sprintf(comResp.message, "211 %d %010d %010d %s", 0, firstArticle, lastArticle, group);	
		else
			sprintf(comResp.message, "211 %d %010d %010d %s", lastArticle-firstArticle+1, firstArticle, lastArticle, group);
		
		addCRLF(comResp.message, COMMAND_SIZE);
		*isGroupSelected = true;
		strcpy(groupSelected, group);
	}
	//Directory does not exist
	else{
		comResp = (CommandResponse){411, "411 No existe ese grupo de noticias."};
		addCRLF(comResp.message, COMMAND_SIZE);
		
		*isGroupSelected = false;
		strcpy(groupSelected, group);
	}

	return comResp;
}



/**
 * ARTICLE command
 */
CommandResponse article(char *command, bool isGroupSelected, char *groupSelected, char ***articleInfo, int *nLines){
	CommandResponse comResp;
	FILE *articleFile, *groupsFile;
	char articleRoute[COMMAND_SIZE], groupRoute[COMMAND_SIZE];
	char aux[COMMAND_SIZE];
	char *row;

	char *article, *subgroup;
	char id[50];
	int numArticle, firstArticle, lastArticle, i;
	bool sintaxError = false;
	regex_t articleRegex;

	//Checks if a group has been selected previously.
	if(!isGroupSelected){
		comResp = (CommandResponse){430, "430 No se encuenta el articulo. No hay grupo seleccionado."};
		addCRLF(comResp.message, COMMAND_SIZE);
		return comResp;
	}

	//If a group is selected checks if command is ok
	if(regcomp(&articleRegex, ARTICLE_REGEX, REG_EXTENDED)){
		perror("ArticleRegex \n");
	}	

	strtok(command, " "); //Discards 'ARTICLE' (name of the command)
	
	//Invalid group
	article = strtok(NULL, " ");
	if(regexec(&articleRegex, article, 0, NULL, 0) == REG_NOMATCH) sintaxError = true;

	//Invalid number of arguments
	if(strtok(NULL, " ") != NULL) sintaxError = true;

	if(sintaxError){
		comResp = (CommandResponse){501, "501 Error de sintaxis en ARTICLE article."};
		addCRLF(comResp.message, COMMAND_SIZE);
		return comResp;
	}


	//Sintax is correct, now parse command
	numArticle = atoi(article);

	//Search the group specified in the groups file (this can be omited if store group info) 
	// and get group route
	if(NULL == (groupsFile = fopen("../noticias/grupos", "r"))){
		comResp = (CommandResponse){-1, "Error"};
		addCRLF(comResp.message, COMMAND_SIZE);
		return comResp;
	}

	while( fgets(aux, COMMAND_SIZE, groupsFile) ){
		if(strcmp(groupSelected, strtok(aux, " ")) == 0){
			lastArticle = atoi(strtok(NULL, " "));
			firstArticle = atoi(strtok(NULL, " "));
			break;
		}				
	}
	fclose(groupsFile);
	
	strcpy(groupRoute, "../noticias/articulos");
	strcpy(aux, "/"); strcat(aux, strtok(strdup(groupSelected), "."));
	strcat(groupRoute, aux);

	while(NULL != (subgroup = strtok(NULL,"."))){
		strcat(groupRoute, "/");
		strcat(groupRoute, subgroup);
	}

	//Check if the number of the article is in the group
	//There are no articles in the group
	if(lastArticle < firstArticle){
		comResp = (CommandResponse){423, "423 No existe el articulo en este grupo de noticias."};
		addCRLF(comResp.message, COMMAND_SIZE);
	
	}
	//There are articles in the group but no one matches with the selected one
	else if(numArticle>lastArticle || numArticle<firstArticle){
		comResp = (CommandResponse){423, "423 No existe el articulo en este grupo de noticias."};
		addCRLF(comResp.message, COMMAND_SIZE);
	}
	//Article is in the group
	else{
		sprintf(articleRoute, "%s/%d", groupRoute, numArticle);

		if(NULL == (articleFile = fopen(articleRoute, "r"))){
			comResp = (CommandResponse){-1, "Error"};
			addCRLF(comResp.message, COMMAND_SIZE);
			return comResp;
		}

		i = 0;
		while( fgets(aux, COMMAND_SIZE, articleFile) ){
			removeCRLF(aux);

			REALLOC_SV( (*articleInfo), (*nLines), (*nLines + 1) )
			(*nLines)++;
			(*articleInfo)[i] = malloc(COMMAND_SIZE * sizeof(char));			
			strcpy( (*articleInfo)[i] , aux);
			addCRLF((*articleInfo)[i], COMMAND_SIZE);
			i++;

			if(aux[0] == '.') break;
			if(aux[0] == '\0') continue;

			row = strtok(aux, ":");
			if(strcmp("Message-ID", row) == 0){
				strcpy(id, strtok(NULL, ":") + 1); //+1 to remove first space	
			}	
		}
		fclose(articleFile);

		comResp = (CommandResponse){223, ""};
		sprintf(comResp.message,"223 %d %s articulo recuperado.", numArticle, id);
		addCRLF(comResp.message, COMMAND_SIZE);
	}

	return comResp;
}





