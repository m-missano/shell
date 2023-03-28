#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>


#define LINE_SIZE 10000

extern char** environ;

char* getCurrentDirectory(){
	char* directory = getenv("PWD");
	char* home = getenv("HOME");
    int home_len = strlen(home);

	//verifica se o diretório atual começa com '/home/user' e se o próximo caractere é '/'
    if (strncmp(directory, home, home_len) == 0 && directory[home_len] == '/') {
        directory[0] = '~';
        memmove(&directory[1], &directory[home_len], strlen(directory) - home_len + 1); 
		//move para o endereço após '~', o diretório após o caminho de home 
    }

	return directory;
}

void type_prompt(){
	struct utsname uts;
	uname(&uts);
	printf("[MySh] %s@%s:%s$ ", getenv("USER"), uts.nodename, getCurrentDirectory());
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

	if(child_pid == 0){
		execvp(program, arglist);
		fprintf(stderr,"%s: comando não encontrado\n",program);
		exit(127);
	} else {
        int child_status;
        waitpid(child_pid, &child_status, 0);

        if (WIFEXITED(child_status)) {
            int exit_status = WEXITSTATUS(child_status);
            if (exit_status != 0) {
                printf("O processo filho terminou com um status de saída anormal: %d\n", exit_status);
            }
        } else {
            printf("O processo filho terminou de forma anormal.\n");
        }
    }

}

int main(int argc, char* argv){
	
	char text_line[LINE_SIZE];
	//char *test[] ={"ls", "-l", NULL};
	int i;

	while(true){ 
		// printa a config inicial do prompt
		type_prompt();	       
		// recebe o comando digitado pelo usuario
		if (fgets(text_line, LINE_SIZE, stdin) == NULL) {
			printf("exit\n");
			exit(0);
		}
		// printa o comando digitado
		text_line[strlen(text_line)-1] = '\0';

		char** args_list = format_text_line(text_line);

		if(args_list[0] != NULL){

			if(strcmp(args_list[0], "help") == 0){
				//print_usage
			}
			else if(strcmp(args_list[0], "cd") == 0){
				//change_directory(args_list);
			}
			else if(strcmp(args_list[0], "exit") == 0){ // sai do programa se receber o comando exit
				break;
			}	
			else{ // comando externo
				spawn(args_list[0], args_list); 
			}
			
			for (i=0; args_list[i] != NULL; i++) {
				free(args_list[i]);
			}
			free(args_list);
		}
	}

	return 0;
}
