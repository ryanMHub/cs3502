#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    	int pipefd[2];
    	pid_t pid;
    	char buffer[100];
    	char *message = "Hello from parent!";

    	//TODO: Create pipe using pipe(pipefd)
	//Check for errors(pipe returns -1 on failure)
    	if (pipe(pipefd) == -1) {
        	perror("pipe");
        	return 1;
    	}

    	//TODO: Fork the process
    	pid = fork();
    	if (pid < 0) {
        	perror("fork");
        	return 1;
    	}

    	if (pid == 0) {
		// Childprocess
		// TODO:Close the write end(child only reads)
        	close(pipefd[1]);

        	// TODO: Read from pipe into buffer
        	ssize_t bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1);
        	if (bytesRead < 0) {
            		perror("read");
            		close(pipefd[0]);
            		return 1;
        	}

	        // Add null terminator to string
        	buffer[bytesRead] = '\0';

	        // TODO: Print the received message
        	printf("Child received: %s\n", buffer);

	        // TODO: Close read end
        	close(pipefd[0]);

	        return 0;
    	} else {
        // Parent process

        // TODO: Close the read end (parent only writes)
        close(pipefd[0]);

        // TODO: Write message to pipe
        ssize_t bytesWritten = write(pipefd[1], message, strlen(message));
        if (bytesWritten < 0) {
            perror("write");
            close(pipefd[1]);
            return 1;
        }

        // TODO: Close write end
        close(pipefd[1]);

        // TODO: Wait for child to finish
        wait(NULL);

        return 0;
    }
}
