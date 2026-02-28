#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

// Configuration - experiment with different values!
#define NUM_ACCOUNTS 8
#define NUM_THREADS 8
#define TRANSACTIONS_PER_THREAD 10
#define INITIAL_BALANCE 80000.0

// Updated Account structure with mutex (GIVEN)
typedef struct {
         int account_id;
         double balance;
         int transaction_count;
         pthread_mutex_t lock; // NEW: Mutex for this account
} Account;

// Global shared array - THIS CAUSES RACE CONDITIONS!
Account accounts[NUM_ACCOUNTS];
//counter to monitor deadlock
volatile int progress_counter = 0;
//count completed threads
volatile int done_count = 0;

//this helper function creates an absolute timeout for pthread_cond_timeout
static void make_abs_timeout(struct timespec* ts, long ms) {
	clock_gettime(CLOCK_REALTIME, ts);
	ts->tv_nsec += ms * 1000000L;
	ts->tv_sec += ts->tv_nsec / 1000000000L;
	ts->tv_nsec %= 1000000000L;
}

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

int safe_transfer_timeout(int from, int to, double amount) {
	if(from == to) return -1;
	if(amount <= 0) return -1;

	struct timespec ts;
	int rc;

	make_abs_timeout(&ts, 10);
	rc = pthread_mutex_timedlock(&accounts[from].lock, &ts);
	if(rc == ETIMEDOUT) return -2;
	if(rc != 0) return -1;

	usleep(100);

	make_abs_timeout(&ts, 2);
	rc = pthread_mutex_timedlock(&accounts[to].lock, &ts);
	if(rc == ETIMEDOUT) {
		pthread_mutex_unlock(&accounts[from].lock);
		return -2;
	}
	if(rc != 0) {
		pthread_mutex_unlock(&accounts[from].lock);
		return -1;
	}

	//Crictical Section
	int result;
	if(accounts[from].balance < amount) {
		result = 0;
	} else {
		accounts[from].balance -= amount;
		accounts[to].balance += amount;
		accounts[from].transaction_count++;
		accounts[to].transaction_count++;
		result = 1;
	}
	// end Critical

	pthread_mutex_unlock(&accounts[to].lock);
	pthread_mutex_unlock(&accounts[from].lock);
	return result;
}

//Get a random amount
double getRandomAmount(unsigned int* seed) {
	return (double)((rand_r(seed) % 100) + 1);
}

//Builds struct to flip flop the accounts to be called by two different threads
//Additionally handles errors and results. Although for this phase you won't see the
//results. Other than proof of deadlock.
void* direct_thread(void* arg) {
	int id = *(int*)arg;

	//generate a seed for the random number generator
	unsigned int seed = time(NULL) ^ (unsigned long)pthread_self();

	for(int i = 0 ; i < TRANSACTIONS_PER_THREAD ; i++) {
		double amount = getRandomAmount(&seed);
		int from = rand_r(&seed) % NUM_ACCOUNTS;
		int to;
		//this is checked here because of the random selection process, but it is also double checked in the safe_transfer_ordered function
		do {
			to = rand_r(&seed) % NUM_ACCOUNTS;
		} while (to == from);

		int rc;
		do {
			rc = safe_transfer_timeout(from, to, amount);
			if(rc == -2) {
				usleep((rand_r(&seed) % 200) + 50);
			}
		} while(rc == -2);

		progress_counter++;

		//display results
		if(rc == 1) {
			printf("Thread %ld: Transfer SUCCESS: $%.2f from %d to %d\n", (long)id, amount, from, to);
		} else if(rc == 0) {
			printf("Thread %ld: Transfer FAILED (insufficient funds): $%.2f from %d to %d\n", (long)id, amount, from, to);
		} else {
			printf("Thread %ld: Transfer ERROR (invalid args): $%.2f from %d to %d\n", (long)id, amount, from, to);
		}
	}

	//update when thread is completed
	done_count++;
	return NULL;
}

//Destroy mutexes AFTER ALL threads complete!
void cleanup_mutexes() {
	for(int i = 0 ; i < NUM_ACCOUNTS; i++) {
		pthread_mutex_destroy(&accounts[i].lock);
	}
}

//Primary application driver
int main() {
       	printf("=== Phase 4: DeadLock Solution Timedlock Demo ===\n\n");

       	initialize_accounts();

       	// Display initial state (GIVEN)
       	printf("Initial State:\n");
       	for (int i = 0; i < NUM_ACCOUNTS; i++) {
              printf(" Account %d: $%.2f\n", i, accounts[i].balance);
       	}

       	double expected_total = NUM_ACCOUNTS * INITIAL_BALANCE;

       	printf("\nExpected total: $%.2f\n\n", expected_total);

	//Declare threads and TransferArgs
       	pthread_t threads[NUM_THREADS];
	int threadID[NUM_THREADS];

	//create threads with a flip flop pattern between accounts
	for(int i = 0 ; i < NUM_THREADS ; i++) {
		threadID[i] = i;
		pthread_create(&threads[i], NULL, direct_thread, &threadID[i]);
	}

	//Stores the starting time of progression observer
	time_t last_time_change = time(NULL);
	//initialize previous state of counter
	int previous_counter = progress_counter;

	//loop until deadlock is reached
	while(done_count < NUM_THREADS) {
		sleep(1); //pause for one second

		if(progress_counter != previous_counter) {
			previous_counter = progress_counter;
			last_time_change = time(NULL);
		}

		if(time(NULL) -	last_time_change >= 5) {
			printf("\n**** Suspected Deadlock - Progress halted for 5 seconds ****\n");
			printf("progress_counter=%d\n", progress_counter);
			break;
		}
	}

	//Display threads completed to user if done_count was successfully reached
	//Join the threads and cleanup_mutexes
	if(done_count == NUM_THREADS) {
		printf("\n**** Threads Completed - Progress was not halted by deadlock ****\n");
		for(int i = 0 ; i < NUM_THREADS ; i++) {
			pthread_join(threads[i], NULL);
		}
		cleanup_mutexes();
	}

	// Calculate and display results
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

        //Check if an error occured in transaction balances
        if(expected_total != actual_total) {
                printf("\nBalances inaccurate\n");
                printf("Run this multiple times - the difference may change each run.\n");
        } else {
                printf("\nBalances are accurate\n");
        }


	return 0;
}


