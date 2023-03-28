#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

// TODO: (se necessario): verificar PWD apos alterar de diretorio
// ! Arrumar os printf (stderr, stdout)

#define LINE_SIZE 10000

extern char** environ;

void format_dir(char* directory){
	char* home = getenv("HOME");
    int home_len = strlen(home);

	//verifica se o diretório atual começa com '/home/user'
    if (strncmp(directory, home, home_len) == 0) {
        directory[0] = '~';
        memmove(&directory[1], &directory[home_len], strlen(directory) - home_len + 1); 
		//move para o endereço após '~', o diretório após o caminho de home 
    }
}

char* getCurrentDirectory(){
	char* aux = getenv("PWD");
	char* directory = (char*)malloc(1024);
	strcpy(directory, aux);
	format_dir(directory);
	return directory;
}

void type_prompt(char* current_dir){
	struct utsname uts;
	uname(&uts);
	printf("[MySh] %s@%s:%s$ ", getenv("USER"), uts.nodename, current_dir);
}

void change_directory(char** arglist, char** current_dir){
	char* path = NULL;
    char* home = getenv("HOME");
    char new_dir[1024];

    if (arglist[1] != NULL) {
        path = arglist[1];
    }
    else {
        path = home;
    }

	if (strcmp(path, "~") == 0) {
		path = home;
	}

	int ret = chdir(path);
	if (ret != 0){
		fprintf(stderr,"bash: cd: %s: No such file or directory\n", path);
		return;
	}

	if (getcwd(new_dir, sizeof(new_dir)) != NULL) {
		//free(current_directory);
		format_dir(new_dir);
		*current_dir = strdup(new_dir); 
	}
	//TODO: tratar caso getwcd não funcione.
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

void ignore_signal(int signum) {
    struct sigaction action;

    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(signum, &action, NULL);
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
	int i;

	char* current_dir = getCurrentDirectory();
	ignore_signal(SIGTSTP);
	ignore_signal(SIGINT);
	while(true){ 
		// printa a config inicial do prompt
		type_prompt(current_dir);	       
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
				change_directory(args_list, &current_dir);
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
