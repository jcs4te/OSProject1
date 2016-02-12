#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <ctype.h>
#include <cstring>

int count = 0;

void parse(char *input, char **args) {
	while (*input != '\0') {
		while (*input == ' ') {
			*input++ = '\0';
		}
		*args++ = input;
		count++;
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
		gets(input);
		printf("\n");
		count = 0;
		parse(input, args);
		for (int i = 0; i < count; i++) {
			printf("%s\n", args[i]);
		}

	}
	return 0;
}