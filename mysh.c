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

struct command {
	char **argv;
};

struct command_list {
    struct command *commands;
    int count;
};

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

struct command_list* format_text_line(char *text_line) {
    struct command_list *cmd_list = malloc(sizeof(struct command_list));
    cmd_list->commands = malloc(sizeof(struct command) * 20);
    cmd_list->count = 0;

    const char delimiter[3] = " \n";
    char* word = strtok(text_line, delimiter);
    struct command current_cmd;
    current_cmd.argv = malloc(sizeof(char*) * 20);
    int arg_count = 0;
	
	while (word != NULL) {
        if (strcmp(word, "|") == 0) {
            current_cmd.argv[arg_count] = NULL;
            cmd_list->commands[cmd_list->count] = current_cmd;
            cmd_list->count++;

            current_cmd.argv = malloc(sizeof(char*) * 20);
            arg_count = 0;
        } else {
            current_cmd.argv[arg_count] = strdup(word);
            arg_count++;
        }

        word = strtok(NULL, delimiter);
    }

    current_cmd.argv[arg_count] = NULL;
    cmd_list->commands[cmd_list->count] = current_cmd;
    cmd_list->count++;

    return cmd_list;
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

pid_t spawn_proc (int in, int out, struct command *cmd){
  	pid_t pid;

  	if ((pid = fork ()) == 0){
      	if (in != 0){
			dup2 (in, 0);
			close (in);
        }

      	if (out != 1){
			dup2 (out, 1);
			close (out);
        }

      return execvp (cmd->argv [0], (char * const *)cmd->argv);
    }

  return pid;
}

void fork_pipes (int n, struct command *cmd){
	int i;
	pid_t pid;
	int in, fd [2];
	
	in = 0;

	for (i = 0; i < n - 1; ++i){
		pipe (fd);
		spawn_proc (in, fd [1], cmd + i);
		close (fd [1]);
		in = fd [0];
	}

	if (in != 0)
		dup2 (in, 0);

	execvp (cmd [i].argv [0], (char * const *)cmd [i].argv);
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

		struct command_list *cmd_list = format_text_line(text_line);

		if(cmd_list->commands[0].argv != NULL){

			if(strcmp(cmd_list->commands[0].argv[0], "help") == 0){
				//print_usage
			}
			else if(strcmp(cmd_list->commands[0].argv[0], "cd") == 0){
				change_directory(cmd_list->commands[0].argv, &current_dir);
			}
			else if(strcmp(cmd_list->commands[0].argv[0], "exit") == 0){ // sai do programa se receber o comando exit
				break;
			}	
			else if(cmd_list->count > 1){ // pipe
				fork_pipes (cmd_list->count, cmd_list->commands);
			}
			else{ // comando externo sem pipe
				spawn(cmd_list->commands[0].argv[0], cmd_list->commands[0].argv);
			}
			
			for (int i = 0; i < cmd_list->count; i++) {
    			free(cmd_list->commands[i].argv);
			}
			free(cmd_list->commands);
			free(cmd_list);
		}
	}

	return 0;
}
