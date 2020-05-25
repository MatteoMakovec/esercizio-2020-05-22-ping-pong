/*
Scrivere un programma che realizza un "ping-pong" tra due processi utilizzando una coppia di pipe, una per ogni direzione.

Il contatore (di tipo int) viene incrementato ad ogni ping ed ad ogni pong e viene trasmesso attraverso la pipe.

Quanto il contatore raggiunge il valore MAX_VALUE il programma termina.

proc_padre manda a proc_figlio il valore 0 attraverso pipeA.
proc_figlio riceve il valore 0, lo incrementa (=1) e lo manda a proc_padre attraverso pipeB.
proc_padre riceve il valore 1, lo incrementa (=2) e lo manda a proc_figlio attraverso pipeA.
proc_figlio riceve il valore 2 .....

fino a MAX_VALUE, quando termina il programma.

#define MAX_VALUE 1000000
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define CHECK_ERR(a,msg) {if ((a) == -1) { perror((msg)); exit(EXIT_FAILURE); } }

#define MAX_VALUE 1000000
#define BUF_SIZE 1024

void child_process();
void parent_process();
void writePipe(int fd, int count);
void readPipe(int fd, int count);

int pipe1[2];
int pipe2[2];
char * buffer;
int pid;

int main() {
	int res = pipe(pipe1);
	CHECK_ERR(res, "pipe1")

	res = pipe(pipe2);
	CHECK_ERR(res, "pipe2")

	pid = getpid();

	int r = 0;

	switch(fork()){
		case 0:
			r = close(pipe2[0]);
			CHECK_ERR(r, "pipe2 close")

			r = close(pipe1[1]);
			CHECK_ERR(r, "pipe1 close")

			child_process();

			r = close(pipe1[0]);
			CHECK_ERR(r, "pipe1 close")

			r = close(pipe2[1]);
			CHECK_ERR(r, "pipe2 close")
			exit(EXIT_SUCCESS);

		case -1:
			perror("fork() error");
			break;

		default:
			r = close(pipe1[0]);
			CHECK_ERR(r, "pipe1 close")

			r = close(pipe2[1]);
			CHECK_ERR(r, "pipe2 close")

			parent_process();

			r = close(pipe2[0]);
			CHECK_ERR(r, "pipe2 close")

			r = close(pipe1[1]);
			CHECK_ERR(r, "pipe1 close")

			if(wait(NULL) == -1){
				perror("wait() error");
				exit(EXIT_FAILURE);
			}
	}

	exit(EXIT_SUCCESS);
}

void parent_process(){
	int counter = 0;
	writePipe(pipe1[1], counter);
	readPipe(pipe2[0], pipe1[1]);
}

void child_process(){
	readPipe(pipe1[0], pipe2[1]);
}

void writePipe(int fd, int count){
	int written = 0;
	int total_written = 0;

	while ((written = write(fd, &count, sizeof(int))) > 0) {
		total_written += written;
		if (total_written < sizeof(int))
			continue;
		else
			return;
	}
}

void readPipe(int fd1, int fd2){
	int count, bytes_read, total_read = 0;
	while ((bytes_read = read(fd1, &count, sizeof(int))) > 0) {
		total_read += bytes_read;
		if (total_read < sizeof(int))
			continue;
		if(count < MAX_VALUE){
			count++;
			if(pid == getpid()){
				printf("[parent] Counter value: %d\n", count);
			}
			else{
				printf("[child] Counter value: %d\n", count);
			}
		}
		else{
			break;
		}

		writePipe(fd2, count);
		total_read = 0;
	}
}
