#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <string.h>

#define LINE_SIZE 10000

void type_prompt(){
	// ! verificar como pegar o nome do usuario
	printf("[MySh] nome-de-usuario@hospedeiro:diretorio-atual$");
}

int main(int argc, char* argv){
	
	char text_line[LINE_SIZE];
	
	while(true){ 
		// printa a config inicial do prompt
		type_prompt();	       
		// recebe o comando digitado pelo usuario
		fgets(text_line, LINE_SIZE, stdin);
		// printa o comando digitado
		printf("%s\n", text_line); // ! executar o comando
		// sai do programa se receber o comando exit
		if(strcmp(text_line, "exit\n") == 0){
			break;
		}	
	}
}
