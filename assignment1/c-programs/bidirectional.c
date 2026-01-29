//Ryan Moskovciak
//CS3502 W03 
//Assignment01 Part 4

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


int main() {
	int pipe1[2]; //Parent to Child
	int pipe2[2]; //Child to Parent
	pid_t pid;

	char parentMsg[] = "Hello from parent!";
	char childMsg[] = "Hello from child!";
	char buffer[200];

	//Create the pipes
	if(pipe(pipe1) == -1) {
		perror("pipe1");
		return 1;
	}
	if(pipe(pipe2) == -1) {
		perror("pipe2");
		return 1;
	}

	//Fork a child
	pid = fork();
	if(pid < 0) {
		perror("fork");
		return 1;
	}

	if(pid == 0) {
		//Child Process
		//close the unused ends
		close(pipe1[1]);
		close(pipe2[0]);

		//Child gets message from parent
		ssize_t n = read(pipe1[0], buffer, sizeof(buffer) -1);
		if(n < 0) {
			perror("child read");
			close(pipe1[0]);
			close(pipe2[1]);
			return 1;
		}
		buffer[n] = '\0';
		printf("Child received: %s\n", buffer);

		//Child sends message to parent
		if(write(pipe2[1], childMsg, strlen(childMsg)) < 0) {
			perror("child write");
			close(pipe1[0]);
			close(pipe2[1]);
			return 1;
		}

		//close the unused ends
		close(pipe1[0]);
		close(pipe2[1]);

		return 0;
	} else {
		//Parent Process
		//Close unused end
		close(pipe1[0]);
		close(pipe2[1]);

		//Parent sends message to child
		if(write(pipe1[1], parentMsg, strlen(parentMsg)) < 0) {
			perror("parent write");
			close(pipe1[1]);;
			close(pipe2[0]);
			return 1;
		}

		//close write end
		close(pipe1[1]);

		//Parent gets message from child
		ssize_t n = read(pipe2[0], buffer, sizeof(buffer) -1);
		if(n < 0) {
			perror("parent read");
			close(pipe2[0]);
			return 1;
		}
		buffer[n] = '\0';
		printf("Parent received: %s\n", buffer);

		close(pipe2[0]);;
		wait(NULL);

		return 0;
	}
}

