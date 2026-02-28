#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Configuration - experiment with different values!
#define NUM_ACCOUNTS 2
#define NUM_THREADS 2
#define INITIAL_BALANCE 1000.0

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
//counter to monitor deadlock
volatile int progress_counter = 0;
//count completed threads
volatile int done_count = 0;

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
//Safe_transfer_ordered prevents deadlock by locking lowest order account first
int safe_transfer_ordered(int from_id, int to_id, double amount) {
	//Check if amount and balances are valid
	if(from_id == to_id) return -1;
	if(amount <= 0) return -1;

	//Determine which account is less than
	int first = (from_id < to_id) ? from_id : to_id;
	int second = (from_id < to_id) ? to_id : from_id;

	//Lock the first account
	pthread_mutex_lock(&accounts[first].lock);
	printf("Thread %ld: Locked account %d\n", (long)pthread_self(), first);

	//Simulate processing delay
	usleep(100);

	//Lock the second account
	printf("Thread %ld: Waiting for account %d\n", (long)pthread_self(), second);
	pthread_mutex_lock(&accounts[second].lock);

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
	pthread_mutex_unlock(&accounts[second].lock);
	pthread_mutex_unlock(&accounts[first].lock);
	return  result;
}

//Builds struct to flip flop the accounts to be called by two different threads
//Additionally handles errors and results. Although for this phase you won't see the
//results. Other than proof of deadlock.
void* deadlock_thread(void* arg) {
	TransferArgs* t = (TransferArgs*)arg;
	int rc = safe_transfer_ordered(t->from, t->to, t->amount);

	//update progress counter and Thread completed counter
	progress_counter++;
	done_count++;

	//display results
	if(rc == 1) {
		printf("Thread %ld: Transfer SUCCESS: $%.2f from %d to %d\n", (long)pthread_self(), t->amount, t->from, t->to);
	} else if(rc == 0) {
		printf("Thread %ld: Transfer FAILED (insufficient funds): $%.2f from %d to %d\n", (long)pthread_self(), t->amount, t->from, t->to);
	} else {
		printf("Thread %ld: Transfer ERROR (invalid args): $%.2f from %d to %d\n", (long)pthread_self(), t->amount, t->from, t->to);
	}

	return NULL;
}

//Destroy mutexes AFTER ALL threads complete!
void cleanup_mutexes() {
	for(int i = 0 ; i < NUM_ACCOUNTS; i++) {
		pthread_mutex_destroy(&accounts[i].lock);
	}
}

//Get a random amount
double getRandomAmount() {
	unsigned int seed = (unsigned int)time(NULL);
	return (double)((rand_r(&seed) % 100) + 1);
}

//Primary application driver
int main() {
       	printf("=== Phase 4: DeadLock Solution Demo ===\n\n");

       	initialize_accounts();

       	// Display initial state (GIVEN)
       	printf("Initial State:\n");
       	for (int i = 0; i < NUM_ACCOUNTS; i++) {
              printf(" Account %d: $%.2f\n", i, accounts[i].balance);
       	}

       	double expected_total = NUM_ACCOUNTS * INITIAL_BALANCE;

       	printf("\nExpected total: $%.2f\n\n", expected_total);

       	pthread_t threads[NUM_THREADS];

	//build flip flopper of accounts
	TransferArgs a = {.from = 0, .to = 1, .amount = getRandomAmount() };
	TransferArgs b = {.from = 1, .to = 0, .amount = getRandomAmount() };

	//create threads 1 and 2 with reversed account calls
	pthread_create(&threads[0], NULL, deadlock_thread, &a);
	pthread_create(&threads[1], NULL, deadlock_thread, &b);

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

	return 0;
}


