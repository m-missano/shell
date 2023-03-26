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
	char *test[] ={"ls", "-l", NULL};
	int child_status;

	while(true){ 
		// printa a config inicial do prompt
		type_prompt();	       
		// recebe o comando digitado pelo usuario
		fgets(text_line, LINE_SIZE, stdin);
		// printa o comando digitado

		text_line[strlen(text_line)-1] = '\0';

		// executa o comando ls
		spawn("ls", test); // ! deixar isso generico

		// espera o processo filho encerrar
		wait(&child_status);
		if(!WIFEXITED(child_status)){
			printf("O processo filho terminou anormalmente\n");
		}

		// sai do programa se receber o comando exit
		if(strcmp(text_line, "exit") == 0){
			break;
		}	
	}

	return 0;
}
