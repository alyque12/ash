#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

void
prompt() {
	char cwd[256];

	getcwd(cwd, 256);
	printf("┌─[aSh][%s]\n└─▪ ", cwd);
}

void
tokenize(char *inbuf, char *cmd[]) {
	char *ptr, *buf;
	int i, size;

	buf = inbuf;

	ptr = strtok(buf, " ");
	i = 0;
	while (ptr != NULL) {
		cmd[i] = ptr;
		ptr = strtok(NULL, " ");
		i++;
	}
	cmd[i] = NULL;
	size = i + 1;
}

void
docd(char *inbuf) {
	char *cmd[256];

	tokenize(inbuf, cmd);

	if (chdir(cmd[1]) == -1) {
		printf("cd: %s: No such file or directory\n", cmd[1]);
	}
}

void
run_exec (char *cmd[]) {
	if (execvp(cmd[0], cmd) == -1) {
		printf("%s: command not found\n", cmd[0]);
		exit(-1);
	}
}

void
do_task(char *inbuf) {
	int i;
	char cmd_string[256], *cmd[256];

	get_cmd(inbuf, cmd_string);

	while (inbuf[i] != '\0') {
		if (inbuf[i] == '>' && inbuf[i+1] == '>') {
			;
		} else if (inbuf[i]) {
			;
		}
		i++;
	}
}

int
main () {
	char inbuf[256], *ptr;

	while(1) {
		prompt();
		fgets(inbuf, 256, stdin);

		ptr = strchr(inbuf, '\n');
		*ptr = '\0';

		if (strncmp(inbuf, "exit", 4) == 0) {
			exit(0);
		} else if (strncmp(inbuf, "cd", 2) == 0) {
			docd(inbuf);
		} else {
			do_task(inbuf);
		}
	}
}
