#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
// Configuration - experiment with different values!
#define NUM_ACCOUNTS 2
#define NUM_THREADS 4
#define TRANSACTIONS_PER_THREAD 10
#define INITIAL_BALANCE 1000.0

// Account data structure (GIVEN)
typedef struct {
	int account_id;
	double balance;
	int transaction_count;
} Account;

// Global shared array - THIS CAUSES RACE CONDITIONS!
Account accounts[NUM_ACCOUNTS];

// GIVEN: Example deposit function WITH race condition
void deposit_unsafe(int account_id, double amount) {

	// READ
	double current_balance = accounts[account_id].balance;

	// MODIFY (simulate processing time)
	//usleep(1); // This increases likelihood of race condition!
	double new_balance = current_balance + amount;

	// WRITE (another thread might have changed balance between READ and WRITE!)
	accounts[account_id].balance = new_balance;
	accounts[account_id].transaction_count++;
}

// TODO 1: Implement withdrawal_unsafe() following the same pattern
// Reference: Copy the structure of deposit_unsafe() above
// Question: What's different between deposit and withdrawal?
void withdrawal_unsafe(int account_id, double amount) {

	// READ
        double current_balance = accounts[account_id].balance;

        // MODIFY (simulate processing time)
        usleep(1); // This increases likelihood of race condition!
        double new_balance = current_balance - amount;

        // WRITE (another thread might have changed balance between READ and WRITE!)
        accounts[account_id].balance = new_balance;
        accounts[account_id].transaction_count++;

}

//TODO This was added since original code didn't actually keep the total dollar amount in the system constant
void transfer_funds(int acc_in, int acc_out, double amount, int teller_id) {
	withdrawal_unsafe(acc_out, amount);
	printf("Teller %d: Withdrew $%.2f from Account %d\n", teller_id, amount, acc_out);

	deposit_unsafe(acc_in, amount);
	printf("Teller %d: Deposited $%.2f to Account %d\n", teller_id, amount, acc_in);
}

// TODO 2: Implement the thread function
// Reference: See OSTEP Ch. 27 for pthread function signature
// Reference: Appendix A.2 for void* parameter explanation
void* teller_thread(void* arg) {
	int teller_id = *(int*)arg; // GIVEN: Extract thread ID

	// TODO 2a: Initialize thread-safe random seed
	//This will generates a new seed for each thread
	unsigned int seed = (unsigned int)(time(NULL)^(unsigned long)pthread_self());
	for (int i = 0; i < TRANSACTIONS_PER_THREAD; i++) {
		// TODO 2b: Randomly select an account (0 to NUM_ACCOUNTS-1)
		//rand_r is used to prevent another race association when threads are competeing for rand() - shared global state
		int acc_in = rand_r(&seed) % NUM_ACCOUNTS;
		int acc_out = rand_r(&seed) % NUM_ACCOUNTS;

		// TODO 2c: Generate random amount (1-100)
		double amount = (double)((rand_r(&seed) % 100) + 1);

		//TODO swap money between accounts
		transfer_funds(acc_in, acc_out, amount, teller_id);
	}
	return NULL;
}

// TODO 3: Implement main function
// Reference: See pthread_create and pthread_join man pages
int main() {
	struct timespec start, end;

	printf("=== Phase 1: Race Conditions Demo ===\n\n");

	// TODO 3a: Initialize all accounts
	// Hint: Loop through accounts array
	// Set: account_id = i, balance = INITIAL_BALANCE, transaction_count = 0

	for(int i = 0 ; i < NUM_ACCOUNTS ; i++) {
		accounts[i].account_id = i;
		accounts[i].balance = INITIAL_BALANCE;
		accounts[i].transaction_count = 0;
	}

	// Display initial state (GIVEN)
	printf("Initial State:\n");
	for (int i = 0; i < NUM_ACCOUNTS; i++) {
		printf(" Account %d: $%.2f\n", i, accounts[i].balance);
	}

	// TODO 3b: Calculate expected final balance
	// Question: With random deposits/withdrawals, what should total be?
	// Hint: Total money in system should remain constant!
	double expected_total = NUM_ACCOUNTS * INITIAL_BALANCE;

	printf("\nExpected total: $%.2f\n\n", expected_total);

	// TODO 3c: Create thread and thread ID arrays
	// Reference: man pthread_create for pthread_t type
	pthread_t threads[NUM_THREADS];
	int thread_ids[NUM_THREADS]; // GIVEN: Separate array for IDs

	// TODO 3d: Create all threads
	// Reference: man pthread_create
	// Caution: See Appendix A.2 warning about passing &i in loop!
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (int i = 0; i < NUM_THREADS; i++) {
		thread_ids[i] = i; // GIVEN: Store ID persistently

		int rc = pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
		if(rc != 0) {
			fprintf(stderr, "Error: pthread_create failed (%d)\n", rc);
			exit(1);
		}
	}

	// TODO 3e: Wait for all threads to complete
	// Reference: man pthread_join
	// Question: What happens if you skip this step?
	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	clock_gettime(CLOCK_MONOTONIC, &end);
	double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
	printf("\n=== Elapsed Time: %.6f seconds\n", elapsed_time);

	// TODO 3f: Calculate and display results
	printf("\n=== Final Results ===\n");
	double actual_total = 0.0;

	for (int i = 0; i < NUM_ACCOUNTS; i++) {
		printf("Account %d: $%.2f (%d transactions)\n",
			i, accounts[i].balance, accounts[i].transaction_count);
		actual_total += accounts[i].balance;
	}

	printf("\nExpected total: $%.2f\n", expected_total);
	printf("Actual total: $%.2f\n", actual_total);
	printf("Difference: $%.2f\n", actual_total - expected_total);

	// TODO 3g: Add race condition detection message
	if(expected_total != actual_total) {
		printf("\nRace Condition Detected\n");
		printf("Run this multiple times - the difference may change each run.\n");
	} else {
		printf("\nNo race detected this run (Run again).\n");
	}

	return 0;
}
