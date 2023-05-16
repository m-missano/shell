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
// ! Arrumar os printf (stderr, stdout) (FEITO)

#define LINE_SIZE 10000

extern char** environ;

struct command {
	char **argv;
};

struct command_list {
    struct command *commands;
    int count;
};

/* formata o diretório atual de acordo com o padrão 
@param directory: Ponteiro para o array de caracteres que representa o diretório atual.
*/
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

// Retorna o diretório atual com a formatação correta. 
char* getCurrentDirectory(){
	char* aux = getenv("PWD");
	char* directory = (char*)malloc(1024);
	strcpy(directory, aux);
	format_dir(directory);
	return directory;
}

/* imprime o prompt no shell seguindo o padrão: 
[MySh] nome-de-usuario@hospedeiro:diretorio-atual$. 
@param current_dir:  diretório atual
*/
void type_prompt(char* current_dir){
	struct utsname uts;
	uname(&uts);
	printf("[MySh] %s@%s:%s$ ", getenv("USER"), uts.nodename, current_dir);
}

/* muda o diretório atual do shell para o especificado pelo usuário ou para o diretório home
@param arglist: um array de strings com os argumentos passados pelo usuário
@param current_dir: um ponteiro para uma string que contém o diretório atual do shell
*/
void change_directory(char** arglist, char** current_dir){
	char* path = NULL;
    char* home = getenv("HOME");
    char new_dir[1024];

	//verifica se o usuário passou um diretório como argumento
    if (arglist[1] != NULL) { 
        path = arglist[1];
    }
    else { //caso não tenha passado, usa o diretório home como padrão
        path = home;
    }

	//caso o diretório seja o "~", usa o diretório home
    if (strcmp(path, "~") == 0) {
        path = home;
    }

	/* Tenta mudar o diretório atual para o especificado, 
	se não conseguir, exibe uma mensagem de erro e sai da função*/
	int ret = chdir(path);
	if (ret != 0){
		fprintf(stderr,"cd: %s: diretório ou arquivo inexistente\n", path);
		return;
	}

	// pega o novo diretório e chama format_dir para formatar seguindo o padrão correto.
	if (getcwd(new_dir, sizeof(new_dir)) != NULL) {
		//free(current_directory);
		format_dir(new_dir);
		*current_dir = strdup(new_dir); //atribui o novo diretório para a variável "current_dir" do shell
	} 
	else {
		fprintf(stderr, "Erro ao obter o diretório atual\n");
		return;
	}
}

/* formata a linha de texto do terminal em uma lista de comandos.
Retorna um ponteiro com a copia formatada da linha do texto com espaços adicionados antes e depois de "|",
possibilitando leitura de pipe sem espaço entre si.
@param text_line: ponteiro para um array de caracteres com a linha do texto
@return char* copy: ponteiro com a copia formatada da linha do texto
*/

char* swap(char *text_line){
 
    char c = 0;
	int pipe_count = 0;
    int length = 0, i = 0, j = 0, length_copy = 0;

    length = strlen(text_line);

	for (i = 0; i < length; i ++) {
		c = text_line[i];
		if (c == '|'){
			pipe_count+=1;
		}
    }
 
	length_copy = strlen(text_line) + pipe_count*2;

	char *copy = (char*)malloc(sizeof(length_copy));
	
	for (i = 0; i < length; i ++) {
		c = text_line[i];
		if (c == '|'){
			copy[j] = ' ';
			copy[j + 1] = '|';
			copy[j + 2] = ' ';
			j+=3;
		}
		else{
			copy[j] = c;
			j++;
		}
    }

	return copy;
}

/* formata a linha de texto do terminal em uma lista de comandos.
Retorna um ponteiro para uma estrutura command_list que contém uma
matriz de estruturas command.
@param text_line: ponteiro para um array de caracteres com a linha do texto*/
struct command_list* format_text_line(char *text_line) {
    struct command_list *cmd_list = malloc(sizeof(struct command_list));
    cmd_list->commands = malloc(sizeof(struct command) * 20);
    cmd_list->count = 0;

	text_line = swap(text_line);
    const char delimiter[2] = " ";
	// separa a linha de texto recebida como parâmetro em palavras, usando o delimitador " ".
    char* word = strtok(text_line, delimiter);
    struct command current_cmd;
    current_cmd.argv = malloc(sizeof(char*) * 20);
    int arg_count = 0;
	
	// loop para percorrer todas as palavras da linha de texto
	while (word != NULL) {
		//verifica se a palavra lida é um caractere de pipe
        if (strcmp(word, "|") == 0) {
			//termina a lista de argumentos do comando atual
            current_cmd.argv[arg_count] = NULL;

			//adiciona o comando atual à lista de comandos
            cmd_list->commands[cmd_list->count] = current_cmd;
            cmd_list->count++;

			// reinicializa a estrutura que armazenará o próximo comando
            current_cmd.argv = malloc(sizeof(char*) * 20);
            arg_count = 0;
        } else {
			// se não leu caractere de pipe, adiciona a palavra atual à lista de arg.
            current_cmd.argv[arg_count] = strdup(word);
            arg_count++;
        }

        word = strtok(NULL, delimiter);
    }
	// termina a lista de argumentos do último comando
    current_cmd.argv[arg_count] = NULL;

	//adiciona o último comando a lista de comandos.
    cmd_list->commands[cmd_list->count] = current_cmd;
    cmd_list->count++;

    return cmd_list;
}

/* Ignora um sinal, cujo número é passado por parâmetro
@param signum: inteiro identificando o sinal a ser ignorado*/
void ignore_signal(int signum) {
    struct sigaction action;

    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(signum, &action, NULL);
}

/* Cria um novo processo filho que executa o programa passado como argumento, 
juntamente com sua lista de argumentos. Aguarda a conclusão do processo filho 
e verifica se ele terminou normalmente. Em caso de saída anormal, exibe uma 
mensagem de erro com o status de saída do processo filho.
@param program: string com o nome do programa a ser executado
@param arglist: array de strings com a lista de argumentos

@return: pid do processo filho criado
*/
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
                fprintf(stderr,"O processo filho terminou com um status de saída anormal: %d\n", exit_status);
            }
        } else {
            fprintf(stderr,"O processo filho terminou de forma anormal.\n");
        }
    }

}

/* Cria um novo processo filho e executa o comando passado como argumento,
redirecionando os descritores de arquivo de entrada e saída se necessário.
@param in: descritor do arq. de entrada
@param out: descritor do arq. de saída
@param cmd: ponteiro para struct com o comando a ser executado e seus args.

@return pid_t: o ID do processo filho criado
*/
pid_t spawn_proc (int in, int out, struct command *cmd){
  	pid_t pid;

  	if ((pid = fork ()) == 0){
		// verifica se há um pipe a ser lido 
      	if (in != 0){
			dup2 (in, 0);
			close (in);
        }
		// verifica se há um pipe a ser escrito
      	if (out != 1){
			dup2 (out, 1);
			close (out);
        }
	//Executa o comando passado
      return execvp (cmd->argv [0], (char * const *)cmd->argv);
    }

  return pid;
}

/* cria um pipe entre cada comando consecutivo, onde é criado um processo
filho para executar cada comando e redirecionar a entrda e saída padrão para 
as extremidades do pipe. O último comando é executado em um processo filho separado.
@param n: inteiro com o número de comandos a serem executados
@param cmd: ponteiro para struct com o comando a ser executado e seus args.
*/
void fork_pipes (int n, struct command *cmd){
    int i;
    pid_t pid;
    int in, fd [2];
    int child_pids[n-1];
    
	// salva o descritor de arquivo original do terminal
	int orig_stdin = dup(STDIN_FILENO);

    in = 0;

    for (i = 0; i < n - 1; ++i){
        pipe (fd);
        pid = spawn_proc (in, fd [1], cmd + i);
        close (fd [1]);
        in = fd [0];
        child_pids[i] = pid;
    }

    if (in != 0)
        dup2 (in, 0);

	// executa o último processo filho
    spawn(cmd [i].argv [0], (char **)cmd [i].argv);
     
    // espera pelo término dos processos filhos
    for (i = 0; i < n - 1; i++) {
        waitpid(child_pids[i], NULL, 0);
    }

	// restaura a entrada padrão do terminal
	if (isatty(orig_stdin)) {
    	dup2(orig_stdin, STDIN_FILENO);
	}

	// fecha o descritor de arquivo original do terminal
	close(orig_stdin);	
}

int main(int argc, char* argv){
	char text_line[LINE_SIZE];
	int i;

	//pega diretório atual
	char* current_dir = getCurrentDirectory();

	//ignora ctrl+c e ctrl+z
	ignore_signal(SIGTSTP);
	ignore_signal(SIGINT);

	//executa o shell enquanto não houver erro ou saída
	while(true){ 
		// printa a config inicial do prompt de comando
		type_prompt(current_dir);	       
		// recebe o comando digitado pelo usuario
		if (fgets(text_line, LINE_SIZE, stdin) == NULL) {
			printf("exit\n");
			exit(0);
		}
		
		text_line[strlen(text_line)-1] = '\0';

		//estrutura de comandos onde cada comando é uma lista com n argumentos
		struct command_list *cmd_list = format_text_line(text_line);
		
		if(cmd_list->commands[0].argv[0] != NULL){

			if(strcmp(cmd_list->commands[0].argv[0], "help") == 0){
				//print_usage
			}
			else if(strcmp(cmd_list->commands[0].argv[0], "cd") == 0){
				change_directory(cmd_list->commands[0].argv, &current_dir);
			}
			else if(strcmp(cmd_list->commands[0].argv[0], "exit") == 0){ // sai do programa se receber o comando exit
				exit(0);
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
			fflush(stdin);
			free(text_line);
		}
	}

	return 0;
}
