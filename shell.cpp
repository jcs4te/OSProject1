#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#include <cstring>

void executer(char **args, char* infile, char* outfile, int a, int b) {
	pid_t pid;
	char *envp[] = {NULL};
	int cur_status, fin, fout;
	pid = fork();
	if (pid < 0) {
		printf("ERROR: Could not fork children\n");
		exit(1);
	} else if (pid == 0) {
		if (a != 0) {
			fin = open(infile, O_RDONLY);
			close(0);
			dup2(fin, 0);
			close(fin);
		}
		if (b != 0) {
			fout = open(outfile, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
			close(1);
			dup2(fout, 1);
			close(fout);
		}
		execvp(*args, args);
		exit(1);
		} else {
			while(wait(&cur_status) != pid);
		}
}

void parse(char *input, char **args) {
	while (*input != '\0') {
		while (*input == ' ') {
			*input++ = '\0';
		}
		*args++ = input;
		while (*input != '\0' && *input != ' ') {
			input++;
		}
	} *args = '\0';
}

int main() {

	char input[80];
	char *args[80];
	char *arg_list[80];
	int fin, fout, stat;
	pid_t pid;
	char *inf = "";
	char *outf = "";

	while(1) {
		printf("$> ");
		fgets(input, 80, stdin);
		int len = strlen(input);
		if( input[len-1] == '\n')
			input[len-1] = 0;
		parse(input, args);
		if (strcmp(args[0], "exit") == 0) {
			printf("Bye\n");
			exit(0);
		}
		arg_list[0] = args[0];
		int i = 1, outflag = 0, inflag = 0;
		while(args[i]) {
			if (strcmp(args[i], ">") == 0) {
				outf = args[i+1];
				outflag = 1;
			} else if (strcmp(args[i], "<") == 0) {
				inf = args[i+1];
				inflag = 1;
			} 

			if (inflag == 0 && outflag == 0) {
				arg_list[i] = args[i];
			}
			i++;
		}
		arg_list[i] = NULL;
		
		executer(arg_list, inf, outf, inflag, outflag);
	}

	return 0;
}