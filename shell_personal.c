#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void
printprompt() {
	char cwd[256];
	char user[256];
	char filepath[256];
	int i, j;

	getcwd(cwd, 256);
	// being extra, not needed
	strcpy(user, getenv("HOME"));

	if (strcmp(cwd, user) == 0) {
		filepath[0] = '~';
	} else {
		filepath[1] = '/';
		for (i = strlen(user) + 1, j = 2; cwd[i] != '\0'; i++, j++) {
			filepath[j] = cwd[i];
		}
		filepath[j] = '\0';
	}
	// stopped being extra
	printf("┌─[aSh][%s]\n└─▪ ", filepath);
}

void tokenize(char *inbuf, char **cmd) {
	char *ptr, *buf;
	int i;

	buf = inbuf;

	ptr = strtok(buf, " ");
	i = 0;
	while (ptr != NULL) {
		cmd[i] = ptr;
		ptr = strtok(NULL, " ");
		i++;
	}
	cmd[i] = NULL;
}

void
dotask(char *inbuf) {
	char **cmd;

	cmd = malloc(256 * 128);
	tokenize(inbuf, cmd);
	if (execvp(cmd[0], cmd) == -1) {
		printf("%s: command not found\n", cmd[0]);
		exit(-1);
	} else {
		exit(0);
	}
}

void
docd(char *inbuf, char **dirs, int *dirs_count) {
	char **cmd;
	char cwd[256];
	char *ptr;

	cmd = malloc(256*128);

	tokenize(inbuf, cmd);

	if (strcmp(cmd[1], "-") == 0) {
		if (*dirs_count < 2) {
			return;
		} else {
			cmd[1] = dirs[*dirs_count - 2];
		}
	} else if (strcmp(cmd[1], "~") == 0) {
		char *home;

		home = malloc(256);
		strcpy(home, getenv("HOME"));
		cmd[1] = home;
	}

	if (chdir(cmd[1]) == -1) {
		printf("cd: %s: No such file or directory\n", cmd[1]);
	} else {
		getcwd(cwd, 256);
		dirs[*dirs_count] = malloc(256);
		strcpy(dirs[*dirs_count], cwd);
		*dirs_count = *dirs_count + 1;
	}
}

int
main() {
	int childpid, status, dirs_count;
	char inbuf[256];
	char cwd[256];
	char **dirs;
	char **cmd;
	char *ptr;

	cmd = malloc(256 * 128);
	dirs_count = 1;
	dirs = malloc(256 * 128);
	dirs[0] = getcwd(cwd, 256);

	while (1) {
		printprompt();
		fgets(inbuf, 256, stdin);
		ptr = strchr(inbuf, '\n');
		*ptr = '\0';
		if (strcmp(inbuf, "exit") == 0) {
			exit(0);
		} else if (strncmp(inbuf, "cd", 2) == 0) {
			docd(inbuf, dirs, &dirs_count);
		} else {
			childpid = fork();
			if (childpid == 0) {
				dotask(inbuf);
			} else {
				wait(&status);
			}
		}
	}
	
}
