#include <stdio.h>
#include <stdbool.h>
#include "unistd.h"
#include <sys/mman.h>
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


typedef enum{
	reading_suc,
	reading_eol,
	reading_wrong_value,
	reading_eof,
} read_rvl_stat;

read_rvl_stat reading_int(int fd, int* cur){
	char c;
	bool negative = false;
	*cur = 0;
	int k = read(fd, &c, sizeof(char));
	if(c == '-'){
		negative = true;
		k = read(fd, &c, sizeof(char));
	}
	while(k > 0){
		if(c == '\n') {
			if(negative) *cur = -(*cur);
			return reading_eol;
		}
		if(c == ' ') break;
		if(((c < '0') || (c > '9'))){
			return reading_wrong_value;
		}
		*cur = *cur * 10 + c - '0';
		k = read(fd, &c, sizeof(char));
	}
	if(negative) *cur = -(*cur);
	if(k == 0) return reading_eof;
	return reading_suc;
}

int main(int argc, char* argv[]){
	if(argc != 2){
	perror("Error in execl\n");
	return -1;
	}
	int cur = 0;
	float first = 0.0f;
	bool fst = true;
	read_rvl_stat status = reading_int(0, &cur);
	char c = '\n';
	while(status != reading_wrong_value){
		if(fst){
			first = cur;
			fst = false;
		} else{
			if (cur == 0) return -3;
			else {
				first = first / cur; 
			}
		}
		if(status == reading_eof){
			fprintf(stderr, "Test should end with <endline>\n");
			return -1;
		} else if(status == reading_eol){
			break;
		}
		cur = 0;
		status = reading_int(0, &cur);
	}
	if (status == reading_wrong_value){
		fprintf(stderr, "Wrong value in test_file \n");
		return -2;
	}
	int desc = open(argv[1], O_RDWR);
	if(desc < 0){
		perror("Tmp file not created\n");
		return -6;
	}
	float* fd = mmap(NULL, sizeof(float),
		PROT_WRITE,
		MAP_SHARED, desc, 0);
	if (fd == MAP_FAILED){
		perror("mmap error\n");
		return -5;
	}
	fd[0] = first;
	if(msync(fd, sizeof(float), MS_SYNC) < 0){
		perror("Msync problem");
		return -6;
	}
	if(munmap(fd, sizeof(float)) < 0){
		perror("Munmap problem");
		return -7;
	}
	close(desc);
	return 0;
}	

