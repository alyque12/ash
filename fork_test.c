#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void
print (char *inbuf) {

	for (int i = 0; i < strlen(inbuf); i++) {
		printf("%c", inbuf[i]);
	}
	printf("\n");
	exit(0);
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
		fgets(inbuf, 256, stdin);
		ptr = strchr(inbuf, '\n');
		*ptr = '\0';
		if (strcmp(inbuf, "exit") == 0) {
			exit(0);
		} else if (strncmp(inbuf, "cd", 2) == 0) {
			;
		} else {
			childpid = fork();
			if (childpid == 0) {
				print(inbuf);
			} else {
				wait(&status);
			}
		}
	}
	
}
