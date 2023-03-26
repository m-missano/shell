#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define LINE_SIZE 10000

void type_prompt(){
	// ! verificar como pegar o nome do usuario
	printf("[MySh] nome-de-usuario@hospedeiro:diretorio-atual$ ");
}

char** format_text_line(char *text){
	int i = 0;
	char* word;
	char** args_list = (char **) malloc(sizeof(char*) * 20); 
	const char delimiter[2] = " ";
	//separa utilizando o delimitador " ", pegando o primeiro token
	word = strtok(text, delimiter);
	
	while (word != NULL) {
		args_list[i] = strdup(word);
		i++;
		word = strtok(NULL, delimiter);
	}
	args_list[i] = NULL;

	return args_list;
}

pid_t spawn(char* program, char** arglist){
	pid_t child_pid = fork();

	if(child_pid != 0){
		return child_pid;
	}
	else{
		execvp(program, arglist);
		abort();
	}

}

int main(int argc, char* argv){
	
	char text_line[LINE_SIZE];
	//char *test[] ={"ls", "-l", NULL};
	int child_status;
	int i;

	while(true){ 
		// printa a config inicial do prompt
		type_prompt();	       
		// recebe o comando digitado pelo usuario
		fgets(text_line, LINE_SIZE, stdin);
		// printa o comando digitado
		text_line[strlen(text_line)-1] = '\0';

		char** args_list = format_text_line(text_line);

		if(strcmp(args_list[0], "help") == 0){
			//print_usage
		}
		else if(strcmp(args_list[0], "cd") == 0){
			//change_directory(params)  
		}
		else if(strcmp(args_list[0], "exit") == 0){ // sai do programa se receber o comando exit
			break;
		}	
		else{ // comando externo
			spawn(args_list[0], args_list); 

			// espera o processo filho encerrar
			wait(&child_status);
			if(!WIFEXITED(child_status)){
				printf("O processo filho terminou anormalmente\n");
			}
		}
		
		for (i=0; args_list[i] != NULL; i++) {
    		free(args_list[i]);
		}
		free(args_list);
	}

	return 0;
}
