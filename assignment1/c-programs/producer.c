//Header files
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

volatile sig_atomic_t shutdown_flag = 0;
volatile sig_atomic_t report_flag = 0;

static unsigned long long lines = 0;
static unsigned long long chars = 0;

//flag function
void handle_signal(int sig) {
	if(sig == SIGINT) shutdown_flag = 1;
	if(sig == SIGUSR1) report_flag = 1;
}

int main(int argc, char *argv[]) {
	char *filename = NULL;
	int buffer_size = 4096;
	int opt;

	//set up signal components
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);

	//command line arguments
	while((opt = getopt(argc, argv, "f:b:")) != -1) {
		switch(opt) {
			case 'f':
				filename = optarg;
				break;
			case 'b':
				buffer_size = atoi(optarg);
				break;
			default:
				fprintf(stderr, "Usage: %s [-f file] [-b size]\n", argv[0]);
				exit(1);
		}
	}

	//if user entered a negative or 0 the buffer size will be corrected
	if(buffer_size <= 0) buffer_size = 4096;

	//check for valid file
	FILE *input = filename ? fopen(filename, "r") : stdin;
	if(!input) {
		perror("fopen");
		return 1;
	}

	//create buffer if not valid exit
	char *buffer = (char *)malloc((size_t)buffer_size);
	if(!buffer) {
		perror("malloc");
		if(filename) fclose(input);
		return 1;
	}

	//main loop
	while(!shutdown_flag) {
		//if signal from user for report display
		if(report_flag) {
			fprintf(stderr, "\n[SIGUSR1] Producer Progress -> Lines: %llu, Chars: %llu\n", lines, chars);
			report_flag = 0;
		}

		//check for end of file and interupt
		errno = 0;
		if(fgets(buffer, buffer_size, input) == NULL) {
			if(errno == EINTR) continue;
			break;
		}

		//increment lines and chars
		lines++;
		for(int i = 0 ; buffer[i] != '\0' ; i++) chars++;

		//pass data to pipeline
		fputs(buffer, stdout);
		fflush(stdout);
	}

	fprintf(stderr, "Producer shutting down gracefully...\n");
	fprintf(stderr, "*** Producer Final Stats ***\nLines: %llu\nChars: %llu\n", lines, chars);
	free(buffer);
	if(filename) fclose(input);

	return 0;

}
