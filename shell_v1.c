#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

void
printprompt() {
	char cwd[256];

	getcwd(cwd, 256);
	printf("┌─[aSh][%s]\n└─▪ ", cwd);
}

void
tokenize(char *inbuf, char *cmd[]) {
	char *ptr, *buf;
	int i, j, read_fd, write_fd, size;

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

char
*middle(char *input, char *output, char *append) {
	if (output < append && output > input) {
		return output;
	} else if (output < input && output > append) {
		return output;
	} else if (input < append && input > output) {
		return input;
	} else if (input < output && input > append) {
		return input;
	} else {
		return append;
	}
}


char
*min(char *input, char *output, char *append) {
	if (input < output) {
		if (input < append) {
			return input;
		} else {
			return append;
		}
	} else {
		if (output < append) {
			return output;
		} else {
			return append;
		}
	}
}

char 
*max(char *input, char *output, char *append) {
	if (input > output) {
		if (input > append) {
			return input;
		} else {
			return append;
		}
	} else {
		if (output > append) {
			return output;
		} else {
			return append;
		}
	}
}

char 
*_strchr(char *inbuf, char c) {
	int i;

	for (i = 0; i < strlen(inbuf); i++) {
		if (inbuf[i] == c) {
			if (inbuf[i-1] == c || inbuf[i+1] == c) {
				continue;
			} else {
				return &inbuf[i];
			}
		}
	}
	return NULL;
}

void
_strncpy (char *dest, char *src, int n) {
	int i;

	dest = malloc(n + 1);
	for (i = 0; i < n; i++) {
		dest[i] = src[i];
	}
	dest[n] = '\0';
}

void
redirection(char *inbuf, char *cmd[]) {
	char *input, *output, *append;
	char string1[256], string2[256], string3[256], string4[256];

	input = _strchr(inbuf, '<');
	output = _strchr(inbuf, '>');
	append = strstr(inbuf, ">>");

	strncpy(string1, inbuf, (min(input, output, append) - inbuf));
	strncpy(string2, min(input, output, append), (middle(input, output, append) - min(input, output, append)));
	strncpy(string3, middle(input, output, append), (max(input, output, append) - middle(input, output, append)));
	strcpy(string4, max(input, output, append));

	printf("%s\n", min(input, output, append));
//	tokenize(string1, cmd);
}

void
dotask(char *inbuf) {
	char *cmd[256];
	char *cmd_string[256];
	int i, size, read_fd, write_fd;

	printf("Child checkpoint 1\n");
	redirection(inbuf, cmd);
	printf("Child checkpoint 2\n");
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
	char *ptr;

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
