#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <time.h>

// Configuration - experiment with different values!
#define NUM_ACCOUNTS 2
#define NUM_THREADS 4
#define TRANSACTIONS_PER_THREAD 10
#define INITIAL_BALANCE 1000.0

void cleanup_mutexes(void);

// Updated Account structure with mutex (GIVEN)
typedef struct {
	int account_id;
	double balance;
	int transaction_count;
	pthread_mutex_t lock; // NEW: Mutex for this account
} Account;

// Global shared array - THIS CAUSES RACE CONDITIONS!
Account accounts[NUM_ACCOUNTS];

// GIVEN: Example of mutex initialization
void initialize_accounts() {
	for (int i = 0; i < NUM_ACCOUNTS; i++) {
		accounts[i].account_id = i;
		accounts[i].balance = INITIAL_BALANCE;
		accounts[i].transaction_count = 0;

		// Initialize the mutex
		pthread_mutex_init(&accounts[i].lock, NULL);
	}
}


// GIVEN: Example deposit function WITH proper protection
void deposit_safe(int account_id, double amount) {
	// Acquire lock BEFORE accessing shared data
	pthread_mutex_lock(&accounts[account_id].lock);

	// ===== CRITICAL SECTION =====
	// Only ONE thread can execute this at a time for this account
	accounts[account_id].balance += amount;
	accounts[account_id].transaction_count++;
	// ============================

	// Release lock AFTER modifying shared data
	pthread_mutex_unlock(&accounts[account_id].lock);
}

// TODO 1: Implement withdrawal_safe() with mutex protection
// Reference: Follow the pattern of deposit_safe() above
// Remember: lock BEFORE accessing data, unlock AFTER
void withdrawal_safe(int account_id, double amount) {
	//Acquire lock BEFORE accessing shared data
	pthread_mutex_lock(&accounts[account_id].lock);

	accounts[account_id].balance -= amount;
	accounts[account_id].transaction_count++;

	//Release lock AFTER modifying shared data
	pthread_mutex_unlock(&accounts[account_id].lock);
}

//TODO This was added since original code didn't actually keep the total dollar amount in the system constant
void transfer_funds(int acc_in, int acc_out, double amount, int teller_id) {
         withdrawal_safe(acc_out, amount);
         printf("Teller %d: Withdrew $%.2f from Account %d\n", teller_id, amount, acc_out);

         deposit_safe(acc_in, amount);
         printf("Teller %d: Deposited $%.2f to Account %d\n", teller_id, amount, acc_in);
}


// TODO 2: Update teller_thread to use safe functions
// Change: deposit_unsafe -> deposit_safe
// Change: withdrawal_unsafe -> withdrawal_safe
void* teller_thread(void* arg) {
        int teller_id = *(int*)arg; // GIVEN: Extract thread ID

        unsigned int seed = (unsigned int)(time(NULL)^(unsigned long)pthread_self());
        for (int i = 0; i < TRANSACTIONS_PER_THREAD; i++) {

                int acc_in = rand_r(&seed) % NUM_ACCOUNTS;
                int acc_out = rand_r(&seed) % NUM_ACCOUNTS;

                double amount = (double)((rand_r(&seed) % 100) + 1);

		transfer_funds(acc_in, acc_out, amount, teller_id);
        }
        return NULL;
}

int main() {
	struct timespec start, end;

        printf("=== Phase 2: Mutex Lock Demo ===\n\n");

        initialize_accounts();

        // Display initial state (GIVEN)
        printf("Initial State:\n");
        for (int i = 0; i < NUM_ACCOUNTS; i++) {
                printf(" Account %d: $%.2f\n", i, accounts[i].balance);
        }

        double expected_total = NUM_ACCOUNTS * INITIAL_BALANCE;

        printf("\nExpected total: $%.2f\n\n", expected_total);

        pthread_t threads[NUM_THREADS];
        int thread_ids[NUM_THREADS]; // GIVEN: Separate array fIDs

        // TODO Add clock_gettime(CLOCK_MONOTONIC, &start)
	clock_gettime(CLOCK_MONOTONIC, &start); 
        for (int i = 0; i < NUM_THREADS; i++) {
                thread_ids[i] = i; // GIVEN: Store ID persistently

                int rc = pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]);
                if(rc != 0) {
                        fprintf(stderr, "Error: pthread_create failed (%d)\n", rc);
                        exit(1);
                }
        }

	for (int i = 0; i < NUM_THREADS; i++) {
        	pthread_join(threads[i], NULL);
        }

	//TODO Add clock_gettime(CLOCK_MONOTONIC, &end) to end the timer
	clock_gettime(CLOCK_MONOTONIC, &end);
	cleanup_mutexes();

	//TODO Calculate time spent using the locking overhead
	double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
	printf("\n=== Elapsed Time: %.6f seconds\n", elapsed_time);

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

// TODO 4: Add mutex cleanup in main()
// Reference: man pthread_mutex_destroy
// Important: Destroy mutexes AFTER all threads complete!
void cleanup_mutexes() {
	for (int i = 0; i < NUM_ACCOUNTS; i++) {
		pthread_mutex_destroy(&accounts[i].lock);
	}
}

