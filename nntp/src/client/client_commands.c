#include "client_commands.h"


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


