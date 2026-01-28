//Header files
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

volatile sig_atomic_t shutdown_flag = 0;
volatile sig_atomic_t report_flag = 0;

void handle_signal(int sig) {
	if(sig == SIGINT) shutdown_flag = 1;
	if(sig == SIGUSR1) report_flag = 1;
}

int main(int argc, char *argv[]) {
	//command line options
	int max_lines = -1;
	int verbose = 0;
	int opt;
	//stores the results
	unsigned long long lines = 0;
	unsigned long long chars = 0;
	//used to buffer content
	char buffer[4096];

	//setup signal handler
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa)); //zero out sa
	sa.sa_handler = handle_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);

	//process command line arguments
	while((opt = getopt(argc, argv, "n:v")) != -1) {
		switch(opt) {
			case 'n':
				max_lines = atoi(optarg);
				break;
			case 'v':
				verbose = 1;
				break;
			default:
				fprintf(stderr, "Usage: %s [-n max] [-v]\n", argv[0]);
				exit(1);
		}
	}

	//main loop of the consumer
	while(!shutdown_flag) {
		//if requested print report
		if(report_flag) {
			fprintf(stderr, "\n[SIGUSR1] Current Progress -> Lines: %llu, Chars: %llu\n", lines, chars);
			report_flag = 0;
		}

		//Check for end of file or interrupt
		errno = 0;
		if(fgets(buffer, sizeof(buffer), stdin) == NULL) {
			if(errno == EINTR) continue; //flag will be caught on next iteration
			break; //EOF
		}

		//increment line count
		lines++;
		//count chars
		for(int i = 0 ; buffer[i] != '\0' ; i++) chars++;

		//display if verbose selected
		if(verbose) {
			printf("%s", buffer);
			fflush(stdout);
		}

		//exit loop if max lines is reached
		if(max_lines > 0 && (int)lines >= max_lines) break;
	}

	//print final statistics to stderr
	fprintf(stderr, "\n**** Final Statistics ****\nLines: %llu\nChars: %llu\n", lines, chars);

	return 0;
}
