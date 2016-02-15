#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <cstring>


void executer(char **args) {
	pid_t pid;
	int cur_status;
	pid = fork();
	if (pid < 0) {
		printf("ERROR: Could not fork children\n");
		exit(1);
	} else if (pid == 0) {
		//printf("\n");
		execvp(*args, args);
		exit(1);
		}
		else {
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
		if (strcmp (args[0], " ") == 0)
			continue;
		/*for (int i = 0; i < count; i++) {
			printf("%s\n", args[i]);
		}*/
		executer(args);

	}

	return 0;
}