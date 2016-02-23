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
#include <vector>
#include <algorithm>
#include <iterator>


void executer(char **args, char* infile, char* outfile, int a, int b, int pipe) {
	int pid;
	char *envp[] = {NULL};
	int cur_status, fin, fout;

	if (pipe == 0)
		pid = fork();
	else {
		pid = 0;
	}
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

void pipe_executer(char **args, char* infile, char* outfile, int a, int b, int c, std::vector<char**> pipe_args) {
	if(c == 1) {
		int pipes[2];
		int pid, cur_status;
		pipe(pipes);
		pid = fork();

		if (pid == 0) {
			dup2(pipes[0], 0);
			close(pipes[1]);
			executer(pipe_args[1], infile, outfile, a, 0, 1);
		} else {
			if (fork() == 0) {
				dup2(pipes[1], 1);
				close(pipes[0]);
				executer(pipe_args[0], infile, outfile, 0, b, 1);
			} else {
				sleep(1);			
			}
		}
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
	int fin, fout, stat;
	pid_t pid;
	char *inf = "";
	char *outf = "";

	while(1) {
		char *args[80];
		char *arg_list[80];
		std::vector<char**> groups;
		int pipecount = 0, count = 0;
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
		int i = 1;
		int outflag = 0, inflag = 0;
		while(args[i]) {
			if (strcmp(args[i], ">") == 0) {
				outf = args[i+1];
				outflag = 1;
				i++;
				break;
			} else if (pipecount == 0 && strcmp(args[i], "<") == 0) {
				inf = args[i+1];
				inflag = 1;
			} else if (strcmp(args[i], "|") == 0) {
				char *arg_temp[80];
				std::copy(arg_list + count, arg_list + i, arg_temp);
				arg_temp[i] = NULL;
				groups.push_back(arg_temp);
				count = i;
				pipecount++;
				i++;
				continue;
			}
			if (inflag == 0 && outflag == 0) {
				arg_list[i-count] = args[i];
			}
			i++;
		}	
		char *arg_temp2[80];

		std::copy(arg_list + count, arg_list + 80, arg_temp2);
		groups.push_back(arg_temp2);

		arg_list[i] = NULL;
		
		if (pipecount == 0) {
			executer(arg_list, inf, outf, inflag, outflag, 0);
		} else {
			pipe_executer(arg_list, inf, outf, inflag, outflag, pipecount, groups);
		}
	}

	return 0;
}