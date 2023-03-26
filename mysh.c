#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <string.h>

void type_prompt(){
	printf("[MySh] nome-de-usuario@hospedeiro:diretorio-atual$");
}

int main(int argc, char* argv){
	
	char* text_line;
	size_t line_size;
	
	while(true){
		type_prompt();
	        getline(&text_line, &line_size, stdin);
		//fgets(text_line);
		printf("%s\n", text_line);
		printf("%d\n", line_size);
		if(strcmp(text_line, "exit\n") == 0){
			break;
		}	
	}
	free(text_line);
}
