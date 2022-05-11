#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdlib.h>

int main() {
	char template[] = "/tmp/tmpXXXXXX";
	int desc = mkstemp(template);
	if(desc < 0){
		perror("Tmp file can not be created\n");
		return -6;
	}
	if(ftruncate(desc, sizeof(float)) < 0){
		perror("Tmp file can not filled\n");
		return -7;
	}
	int count;
	char num;
	while (read(0, &num, sizeof(char)) > 0) {
		if (num == ' ') break;
		count = count * 10 + (num - '0');
	}
	char name[count + 1];
	read (0, &name, count * sizeof(char));
	name[count] = '\0';

	int fd[2];
	if (pipe(fd) < 0) {
	return -1; 
	}

	int file = open(name, O_RDONLY);
	if (file < 0){
		return -3;
	}
	int processID = fork();
	if (processID == 0){
		dup2(file, 0);
		execl("child", "child", template, NULL);
	} else {
		int status;
		wait(&status);
		if (WEXITSTATUS(status)){
			return 2;
		}
		unlink(template);
		float* fd = mmap(NULL, sizeof(float),
			PROT_READ | PROT_WRITE,
			MAP_SHARED, desc, 0);
		if (fd == MAP_FAILED){
			perror("mmap error\n");
			return -5;
		}
		printf("%f\n", fd[0]);
		if(munmap(fd, sizeof(float)) < 0){
			perror("Munmap problem");
			return -6;
		}
		close(desc);
	}
	return 0; 
}


