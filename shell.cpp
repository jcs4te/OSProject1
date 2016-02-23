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
	// printf("%s\n", pipe_args[0][0]);
	// printf("%s\n", pipe_args[0][1]);
	// printf("%s\n", pipe_args[1][0]);
	// printf("%s\n", pipe_args[1][1]);	
	// printf("%s\n", pipe_args[2][0]);
	// printf("%s\n", pipe_args[2][1]);

	/* cat output.txt | grep n | grep a */

	if(c == 1) {
		int pipes[2];
		int pid, cur_status, pid2;
		pipe(pipes);
		pid = fork();

		if (pid == 0) {
			dup2(pipes[0], 0);
			close(pipes[1]);
			executer(pipe_args[1], infile, outfile, a, 0, 1);
		} else {
			if ((pid2 = fork()) == 0) {
				dup2(pipes[1], 1);
				close(pipes[0]);
				executer(pipe_args[0], infile, outfile, 0, b, 1);
			} else {
				while(wait(&cur_status) != pid2);		
			}
			sleep(1);
		}
	} else if(c == 2) {
		int pipes[4];
		int status;

		pipe(pipes);
		pipe(pipes + 2);

		if(fork() == 0) {
			dup2(pipes[1], 1);
			close(pipes[0]);
			close(pipes[1]);
			close(pipes[2]);
			close(pipes[3]);
			executer(pipe_args[0], infile, outfile, a, 0, 1);
		} else {
			if(fork() == 0) {
				dup2(pipes[0], 0);
				dup2(pipes[3], 1);

				close(pipes[0]);
				close(pipes[1]);
				close(pipes[2]);
				close(pipes[3]);

				executer(pipe_args[1], infile, outfile, 0, 0, 1);
			} else {
				if(fork() == 0) {
					dup2(pipes[2], 0);

					close(pipes[0]);
					close(pipes[1]);
					close(pipes[2]);
					close(pipes[3]);

					executer(pipe_args[2], infile, outfile, 0, b, 1);
				}
			}
		}

		close(pipes[0]);
		close(pipes[1]);
		close(pipes[2]);
		close(pipes[3]);

		for(int i = 0; i < 3; i++) {
			wait(&status);
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
	int pid;
	char *inf = "";
	char *outf = "";
	int pid2, pid3, cur_status;

	while(1) {
		char *args[80];
		char *arg_list[80];

		int haveweseen_I = 0;
		int haveweseen_O = 0;
		int haveweseen_P = 0;
		int errorcheck = 0;
		
		for(int i = 0; i < 80; i++) {
			arg_list[i] = '\0';
		}

		arg_list[80] = NULL;

		std::vector<char**> groups;
		int pipecount = 0, count = 0;
		printf("$> ");
		fgets(input, 82, stdin);
		int len = strlen(input);
		if(len > 80) {
			printf("%s\n", "***ERROR***: Input over 80 chars");
			continue;
		}
		if( input[len-1] == '\n')
			input[len-1] = 0;
		parse(input, args);
		if (strcmp(args[0], "exit") == 0) {
			printf("Bye\n");
			exit(0);
		}
		int i = 0;
		int outflag = 0, inflag = 0;
		
		while(args[i]) {
			if (strcmp(args[i], ">") == 0) {
				if (haveweseen_O || strcmp(args[i+1], ">") == 0 ) {
					printf("%s\n", "***ERROR***: invalid operator");
					errorcheck = 1;
					break;
				}
				haveweseen_O = 1;
				outf = args[i+1];
				outflag = 1;
				i++;
			} else if (pipecount == 0 && strcmp(args[i], "<") == 0) {
				if (haveweseen_I || haveweseen_O || haveweseen_P || strcmp(args[i+1], "<") == 0 ) {
					printf("%s\n", "***ERROR***: invalid operator");
					errorcheck = 1;
					break;
				}
				inf = args[i];
				inflag = 1;
			} else if (strcmp(args[i], "|") == 0) {
				if (haveweseen_O || strcmp(args[i+1], "|") == 0 ) {
					printf("%s\n", "***ERROR***: invalid operator");	
					errorcheck = 1;
					break;
				}
				if(pipecount == 0) {
					char *arg_temp[80];
					std::copy(arg_list, arg_list + i, arg_temp);
					arg_temp[i] = NULL;
					groups.push_back(arg_temp);
				} else if (pipecount == 1) {
					char *arg_temp2[80];
					std::copy(arg_list + 1, arg_list + i, arg_temp2);
					arg_temp2[i] = NULL;
					groups.push_back(arg_temp2);
				}
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
		if (errorcheck) continue;

		char *arg_temp3[80];

		std::copy(arg_list + 1, arg_list + 80, arg_temp3);
		arg_temp3[i] = NULL;
		groups.push_back(arg_temp3);

		arg_list[i] = NULL;
		
		if (pipecount == 0) {
			executer(arg_list, inf, outf, inflag, outflag, 0);
		} else {
			pipe_executer(arg_list, inf, outf, inflag, outflag, pipecount, groups);
		}
	}

	return 0;
}