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
	// YOUR CODE HERE
	// Hint: pthread_mutex_lock
	// Hint: Modify balance
	// Hint: pthread_mutex_unlock
}

// TODO 2: Update teller_thread to use safe functions
// Change: deposit_unsafe -> deposit_safe
// Change: withdrawal_unsafe -> withdrawal_safe
void* teller_thread(void* arg) {
        int teller_id = *(int*)arg; // GIVEN: Extract thread ID

        unsigned int seed = (unsigned int)(time(NULL)^(unsigned long)pthread_se>
        for (int i = 0; i < TRANSACTIONS_PER_THREAD; i++) {

                int account_idx = rand_r(&seed) % NUM_ACCOUNTS;

                double amount = (double)((rand_r(&seed) % 100) + 1);


                int operation = rand_r(&seed) % 2;

                if (operation == 1) {
                        deposit_unsafe(account_idx, amount);
                        printf("Teller %d: Deposited $%.2f to Account %d\n",
                                teller_id, amount, account_idx);
                } else {
                        withdrawal_unsafe(account_idx, amount);
                        printf("Teller %d: Withdrew $%.2f from Account %d\n",
                                teller_id, amount, account_idx);
                }
        }
        return NULL;
}

// TODO 3: Add performance timing
// Reference: Section 7.2 "Performance Measurement"
// Hint: Use clock_gettime(CLOCK_MONOTONIC, &start);
int main() {
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
        //      pthread_join(threads[i], NULL);
        }

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
                printf("Run this multiple times - the difference may change eac>
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

// TODO 5: Compare Phase 1 vs Phase 2 performance
// Measure execution time for both versions
// Document the overhead of synchronization
