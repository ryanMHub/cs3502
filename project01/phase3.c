#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <time.h>

// Configuration - experiment with different values!
#define NUM_ACCOUNTS 2
#define NUM_THREADS 2
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

//Struct to handle thread flip to test deadlock
typedef struct {
	int from;
	int to;
	double amount;
} TransferArgs;

// Global shared array - THIS CAUSES RACE CONDITIONS!
Account accounts[NUM_ACCOUNTS];

//Initialize each accounts values and mutex
void initialize_accounts() {
        for (int i = 0; i < NUM_ACCOUNTS; i++) {
                accounts[i].account_id = i;
                accounts[i].balance = INITIAL_BALANCE;
                accounts[i].transaction_count = 0;
                // Initialize the mutex
                pthread_mutex_init(&accounts[i].lock, NULL);
        }
}

//Important return values 1 = Transfer successful, 0 = Transfer failure, -1 = error handling
int transfer_deadlock(int from_id, int to_id, double amount) {
	//Check if amount and balances are valid
	if(from_id == to_id) return -1;
	if(amount <= 0) return -1;

	//Lock source account
	pthread_mutex_lock(&accounts[from_id].lock);
	printf("Thread %ld: Locked account %d\n", (long)pthread_self(), from_id);

	//Simulate processing delay
	usleep(100);

	//Try to lock destination account
	printf("Thread %ld: Waiting for account %d\n", (long)pthread_self(), to_id);
	pthread_mutex_lock(&accounts[to_id].lock);

	int result;

	//Check if there is enough money in the from account before conducting transfer
	if(accounts[from_id].balance < amount) {
		result = 0;
	} else {
		//Transfer (never reached if deadlocked)
		accounts[from_id].balance -= amount;
		accounts[to_id].balance += amount;
		accounts[from_id].transaction_count++;
		accounts[to_id].transaction_count++;
		result = 1;
	}

	//Unlock the locked accounts
	pthread_mutex_unlock(&accounts[to_id].lock);
	pthread_mutex_unlock(&accounts[from_id].lock);
	return  result;
}

//Builds struct to flip flop the accounts to be called by two different threads
//Additionally handles errors and results. Although for this phase you won't see the
//results. Other than proof of deadlock.
void* deadlock_thread(void* arg) {
	TransferArgs* t = (TransferArgs*)arg;
	int rc = transfer_deadlock(t->from, t->to, t->amount);

	if(rc == 1) {
		printf("Thread %ld: Transfer SUCCESS: $%.2f from %d to %d\n", (long)pthread_self(), t->amount, t->from, t->to);
	} else if(rc == 0) {
		printf("Thread %ld: Transfer FAILED (insufficient funds): $%.2f from %d to %d\n", (long)pthread_self(), t->amount, t->from, t->to);
	} else {
		printf("Thread %ld: Transfer ERROR (invalid args): $%.2f from %d to %d\n", (long)pthread_self(), t->amount, t->from, t->to);
	}

	return NULL;
}

//Get a random amount
double getRandomAmount() {
	unsigned int seed = (unsigned int)time(NULL);
	return (double)((rand_r(&seed) % 100) + 1);
}

//Primary application driver
int main() {
       	struct timespec start, end;

       	printf("=== Phase 3: DeadLock Demo ===\n\n");

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

	//build flip flopper of accounts
	TransferArgs a = {.from = 0, .to = 1, .amount = getRandomAmount() };
	TransferArgs b = {.from = 1, .to = 0, .amount = getRandomAmount() };

	//create threads 1 and 2 with reversed account calls
	pthread_create(&threads[0], NULL, deadlock_thread, &a);
	pthread_create(&threads[1], NULL, deadlock_thread, &b);


       	// TODO Add clock_gettime(CLOCK_MONOTONIC, &start)
       	clock_gettime(CLOCK_MONOTONIC, &start);


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
