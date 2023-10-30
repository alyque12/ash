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

void
get_filename(char *filename, char *inbuf, int i, int j) {
	int k;

	k = 0;

	while (1) {
		if (inbuf[i+j] == '\0') {
			break;
		} else if (inbuf[i+j] == '>' && inbuf[i+1+j] == '>') {
			break;
		} else if (inbuf[i+j] == '>' && inbuf[i+j+1] != '>') {
			break;
		} else if (inbuf[i+j] == ' ') {
			j++;
		} else if (inbuf[i+j] == '<') {
			break;
		} else {
			filename[k] = inbuf[i+j];
			k++;
			j++;
		}
	}
	filename[k] = '\0';
}

void
redirection (char *inbuf, char *cmd[]) {
	int cmd_parsed, i, j, fd, k;
	char inbuf_cpy[256];
	char cmd_string[256];
	char filename[256];

	strcpy(inbuf_cpy, inbuf);

	i = 0;
	k = 0;
	cmd_parsed = 0;
	while (inbuf[i] != '\0') {
		if (inbuf[i] == '<') {
			cmd_parsed = 1;
			j = 1;
			get_filename(filename, inbuf_cpy, i, j);
			fd = open(filename, O_RDONLY);
			close(0);
			dup(fd);
		} else if (inbuf[i] == '>' && inbuf[i+1] == '>') {
			cmd_parsed = 1;
			j = 2;
			get_filename(filename, inbuf_cpy, i, j);
			fd = open(filename, O_WRONLY | O_APPEND | O_CREAT);
			close(1);
			dup(fd);
		} else if (inbuf[i] == '>' && inbuf[i+1] != '>' && inbuf[i-1] != '>') {
			cmd_parsed = 1;
			j = 1;
			get_filename(filename, inbuf_cpy, i, j);
			fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC);
			close(1);
			dup(fd);
		}
		if (cmd_parsed == 0) {
			cmd_string[i] = inbuf_cpy[i];
			k++;
		}

		i++;
	}
	cmd_string[k] = '\0';
	tokenize(cmd_string, cmd);
}

void
dotask(char *inbuf) {
	char *cmd[256];

	redirection(inbuf, cmd);
	if (execvp(cmd[0], cmd) == -1) {
		printf("%s: command not found\n", cmd[0]);
		exit(-1);
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
pipe_parse(char *inbuf, int bookmark, char *cmd) {
	char inbuf_cpy[256];
	int i;

	i = 0;
	strcpy(inbuf_cpy, inbuf);
	while (inbuf_cpy[bookmark] != '\0') {
		if (inbuf_cpy[bookmark] == '|') {
			cmd[i] = '\0';
			bookmark = bookmark + 1;
			return bookmark;
		} else {
			cmd[i] = inbuf_cpy[bookmark];
			i++;
			bookmark = bookmark + 1;
		}
	}
	cmd[i] = '\0';
	return -1;
}

void
pipe_task(char *inbuf, int pipe_count) {
	int fd[pipe_count][2], bookmark, i, status, childpid, write, read;
	char cmd_string[256];

	for (i = 0; i < pipe_count; i++) {
		pipe(fd[i]);
	}
	bookmark = 0;
	read = 0;
	write = 0;
	for (i = 0; i < pipe_count + 1; i++) {
		bookmark = pipe_parse(inbuf, bookmark, cmd_string);
		childpid = fork();
		if (childpid == 0) {
			printf("child reached\n");
			if (i - 1 >= 0) {
				printf("i: %d cmd: %s\n", i, cmd_string);
				read = 1;
				dup2(fd[i-1][0], STDIN_FILENO);
				close(fd[i-1][0]);
				close(fd[i-1][1]);
			}
			if (i + 1 <= pipe_count) {
				printf("i: %d cmd: %s\n", i, cmd_string);
				write = 1;
				dup2(fd[i][1], STDOUT_FILENO);
				close(fd[i][1]);
				close(fd[i][0]);
			}
			dotask(cmd_string);
		} else {
			if (read == 1) {
				close(fd[i - 1][0]);
			}
			if (write == 1) {
				close(fd[i][1]);
			}
			read = 0;
			write = 0;
			wait(&status);
		}
		/*
		if (i == 0) {
			childpid = fork();
			if (childpid == 0) {
				dup2(fd[1], STDOUT_FILENO);
				close(fd[0]);
				close(fd[1]);
				dotask(cmd_string);
			} else {
				close(fd[1]);
				wait(&status);
			}
		} else {
			childpid = fork();
			if (childpid == 0){
				dup2(fd[0], STDIN_FILENO);
				close(fd[0]);
				close(fd[1]);
				dotask(cmd_string);
			} else {
				close(fd[0]);
				wait(&status);
			}
		}
		*/
	}
}

int
main() {
	int childpid, status, dirs_count, i, pipe_count;
	char inbuf[256];
	char cmd_string[256];
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
			i = 0;
			pipe_count = 0;
			while (inbuf[i] != '\0') {
				if (inbuf[i] == '|') {
					pipe_count++;
				}
				i++;
			}
			if (pipe_count > 0) {
				pipe_task(inbuf, pipe_count);
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
	
}
