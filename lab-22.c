#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define ERROR_TREAD_CREATE -1
#define ERROR_TREAD_JOIN -2
#define ERROR_SEMAPHORE_WAIT -3
#define ERROR_SEMAPHORE_POST -4
#define ERROR_SEMAPHORE_INIT -5
#define ERROR_SEMAPHORE_DESTROY -6
#define ALL_RIGHT 0
#define ERROR_MUTEX_CREATE -5
#define ERROR_MUTEX_INIT -7
#define ERROR_CONDITION_CREATE -6
#define ERROR_MUTEX_DESTROY -6

#define MAX_A 10
#define MAX_B 5
#define MAX_C 2
#define MAX_AB 3
#define MAX_WIDGET 10


void printTreadError(int errCode, char * comment) {
	char *errorLine = strerror(errCode);
	fprintf(stderr, "%s: %s\n", comment, errorLine);
}

void pthread_cancel_wrapper(const pthread_t* pthread) {
    int err_code = pthread_cancel(*pthread);
    if (err_code != ALL_RIGHT) {
        printTreadError(err_code, "error pthread_cancel");
    }
}


volatile char is_stopped = 0;

sem_t sem_A, sem_B, sem_C, sem_mAB;
pthread_cond_t cond_A, cond_B, cond_C, cond_mAB;
pthread_mutex_t mut_A, mut_B, mut_C, mut_mAB;

int counter_A = 0,
counter_B = 0,
counter_C = 0,
counter_AB = 0,
counter_W = 0;
//max sem value
//delete counters
//optimize count of cargo  - if cargo full - find desigeon 

void *createDetailA(void *arg) {
	int error;

	while (!is_stopped) {
		int A;
		sem_getvalue(&sem_A, &A);

		if(A != MAX_A){
			sleep(1);
			error = sem_post(&sem_A);
			if (error != ALL_RIGHT) {
				printTreadError(error, "error semaphore post");
				exit(ERROR_SEMAPHORE_POST);
			}
			printf("Created detail A\n");
			counter_A++;
		} else {
			printf("Cargo A is full. Wait\n");
			pthread_mutex_lock(&mut_A);
			pthread_cond_wait(&cond_A, &mut_A);
			pthread_mutex_unlock(&mut_A);
		}
	}
	pthread_exit(NULL);
}

void *createDetailB(void *arg) {
	int error;
	while (!is_stopped) {
		int B;
		sem_getvalue(&sem_B, &B);
		if(B != MAX_B){
			sleep(2);
			error = sem_post(&sem_B);
			if (error != ALL_RIGHT) {
				printTreadError(error, "error semaphore post");
				exit(ERROR_SEMAPHORE_POST);
			}
			printf("Created detail B\n");
			counter_B++;
		} else {
			printf("Cargo B is full. Wait\n");
			pthread_mutex_lock(&mut_B);
			pthread_cond_wait(&cond_B, &mut_B);
			pthread_mutex_unlock(&mut_B);
		}
	}
	pthread_exit(NULL);
}

void *createDetailC(void *arg) {
	int error;
	while (!is_stopped) {
		int C;
		sem_getvalue(&sem_C, &C);
		if(C != MAX_C){
			sleep(3);
			error = sem_post(&sem_C);
			if (error != ALL_RIGHT) {
				printTreadError(error, "error semaphore post");
				exit(ERROR_SEMAPHORE_POST);
			}
			printf("Created detail C\n");
			counter_C++;
			
		} else {
			printf("Cargo C is full. Wait\n");
			pthread_mutex_lock(&mut_C);
			pthread_cond_wait(&cond_C, &mut_C);
			pthread_mutex_unlock(&mut_C);
		}
	}
	pthread_exit(NULL);
}

void *createModuleAB(void *arg) {
	int error;
	while (!is_stopped) {
		int AB;
		sem_getvalue(&sem_mAB, &AB);
		if(AB != MAX_AB){
			
			error = sem_wait(&sem_A);
			pthread_cond_signal(&cond_A);
			if (error != ALL_RIGHT) {
				printTreadError(error, "error semaphore wait");
				exit(ERROR_SEMAPHORE_WAIT);
			}
			error = sem_wait(&sem_B);
			pthread_cond_signal(&cond_B);
			if (error != ALL_RIGHT) {
				printTreadError(error, "error semaphore wait");
				exit(ERROR_SEMAPHORE_WAIT);
			}

			error = sem_post(&sem_mAB);
			if (error != ALL_RIGHT) {
				printTreadError(error, "error semaphore post");
				exit(ERROR_SEMAPHORE_POST);
			}

			printf("Created module AB\n");
			counter_AB++;
		
		} else {
			printf("Cargo AB is full. Wait\n");
			pthread_mutex_lock(&mut_mAB);
			pthread_cond_wait(&cond_mAB, &mut_mAB);
			pthread_mutex_unlock(&mut_mAB);
		}
	}
	pthread_exit(NULL);
}

void *createWidget(void *arg) {
	int error;
	while (!is_stopped) {
		
		if(counter_W != MAX_WIDGET){
			error = sem_wait(&sem_C);
			pthread_cond_signal(&cond_C);
			if (error != ALL_RIGHT) {
				printTreadError(error, "error semaphore wait");
				exit(ERROR_SEMAPHORE_WAIT);
			}

			error = sem_wait(&sem_mAB);
			pthread_cond_signal(&cond_mAB);
			if (error != ALL_RIGHT) {
				printTreadError(error, "error semaphore wait");
				exit(ERROR_SEMAPHORE_WAIT);
			}
			printf("Created widget\n");
			counter_W++;
		} else {
			printf("Cargo wigit is full. Finishing the program\n");
			is_stopped = 1;
			pthread_cond_signal(&cond_A);
			pthread_cond_signal(&cond_B);
			pthread_cond_signal(&cond_C);
			pthread_cond_signal(&cond_mAB);
			pthread_exit(NULL);
			
		}
	}
	pthread_exit(NULL);
}


void sig_handler(int sig) {
	is_stopped = 1;
}

int init_sems() {
	int error;
	
	error = pthread_mutex_init (&mut_A, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't create mutex");
		return ERROR_MUTEX_INIT;
	}
	error = pthread_mutex_init (&mut_B, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't create mutex");
		return ERROR_MUTEX_INIT;
	}
	
	error = pthread_mutex_init (&mut_C, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't create mutex");
		return ERROR_MUTEX_INIT;
	}
	
	error = pthread_mutex_init (&mut_mAB, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't create mutex");
		return ERROR_MUTEX_INIT;
	}
	
	
	error = pthread_cond_init(&cond_A, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't create condition");
		return ERROR_CONDITION_CREATE;
	}
	error = pthread_cond_init(&cond_B, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't create condition");
		return ERROR_CONDITION_CREATE;
	}
	error = pthread_cond_init(&cond_C, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't create condition");
		return ERROR_CONDITION_CREATE;
	}
	error = pthread_cond_init(&cond_mAB, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't create condition");
		return ERROR_CONDITION_CREATE;
	}
	
	error = sem_init(&sem_A, 0, 0);
	if (error != ALL_RIGHT) {
		printTreadError(error, "can't init semaphore");
		return ERROR_SEMAPHORE_INIT;
	}
	error = sem_init(&sem_B, 0, 0);
	if (error != ALL_RIGHT) {
		printTreadError(error, "can't init semaphore");
		return ERROR_SEMAPHORE_INIT;
	}
	error = sem_init(&sem_C, 0, 0);
	if (error != ALL_RIGHT) {
		printTreadError(error, "can't init semaphore");
		return ERROR_SEMAPHORE_INIT;
	}
	error = sem_init(&sem_mAB, 0, 0);
	if (error != ALL_RIGHT) {
		printTreadError(error, "can't init semaphore");
		return ERROR_SEMAPHORE_INIT;
	}
	
	
	return ALL_RIGHT;
}

int destroy_sems() {
	int error;
	error = sem_destroy(&sem_A);
	if (error != ALL_RIGHT) {
		printTreadError(error, "can't destroy semaphore");
		return ERROR_SEMAPHORE_DESTROY;
	}
	error = sem_destroy(&sem_B);
	if (error != ALL_RIGHT) {
		printTreadError(error, "can't destroy semaphore");
		return ERROR_SEMAPHORE_DESTROY;
	}
	error = sem_destroy(&sem_C);
	if (error != ALL_RIGHT) {
		printTreadError(error, "can't destroy semaphore");
		return ERROR_SEMAPHORE_DESTROY;
	}
	error = sem_destroy(&sem_mAB);
	if (error != ALL_RIGHT) {
		printTreadError(error, "can't destroy semaphore");
		return ERROR_SEMAPHORE_DESTROY;
	}
	
		error = pthread_mutex_destroy(&mut_A);
	if (error != ALL_RIGHT) {
		printTreadError(error, "mutex destroy error");
		return ERROR_MUTEX_DESTROY;
	}
	error = pthread_mutex_destroy(&mut_B);
	if (error != ALL_RIGHT) {
		printTreadError(error, "mutex destroy error");
		return ERROR_MUTEX_DESTROY;
	}
	error = pthread_mutex_destroy(&mut_C);
	if (error != ALL_RIGHT) {
		printTreadError(error, "mutex destroy error");
		return ERROR_MUTEX_DESTROY;
	}
	error = pthread_mutex_destroy(&mut_mA]);
	if (error != ALL_RIGHT) {
		printTreadError(error, "mutex destroy error");
		return ERROR_MUTEX_DESTROY;
	}
	
	return ALL_RIGHT;
}


int  work(){
	int error;
	pthread_t thread[5];
	error = pthread_create(&thread[0], NULL, createDetailA, NULL);
	if (ALL_RIGHT != error) {
		printTreadError(error, "creation thread(A) error");
		return ERROR_TREAD_CREATE;
	}
	error = pthread_create(&thread[1], NULL, createDetailB, NULL);
	if (ALL_RIGHT != error) {
		printTreadError(error, "creation thread(B) error");
		return ERROR_TREAD_CREATE;
	}
	error = pthread_create(&thread[2], NULL, createDetailC, NULL);
	if (ALL_RIGHT != error) {
		printTreadError(error, "creation thread(C) error");
		return ERROR_TREAD_CREATE;
	}
	error = pthread_create(&thread[3], NULL, createModuleAB, NULL);
	if (ALL_RIGHT != error) {
		printTreadError(error, "creation thread(AB) error");
		return ERROR_TREAD_CREATE;
	}
	error = pthread_create(&thread[4], NULL, createWidget, NULL);
	if (ALL_RIGHT != error) {
		printTreadError(error, "creation thread(W) error");
		return ERROR_TREAD_CREATE;
	}

	sigset(SIGINT, sig_handler);

	for (int i = 0; i < 5; ++i) {
		error = pthread_join(thread[i], NULL);
		if (ALL_RIGHT != error) {
			printTreadError(error, "join thread error");
		}
	}
	
}

int main(int argC, char **argV) {

	int error;

	error = init_sems();
	if (error != ALL_RIGHT) {
		return ERROR_SEMAPHORE_INIT;
	}

	error = work();
	if (error != ALL_RIGHT) {
		return error;
	}
	
	error = destroy_sems();
	if (error != ALL_RIGHT) {
		return ERROR_SEMAPHORE_DESTROY;
	}
	
	printf("#Created : \n Detail  A: %d\n Detail  B: %d\n Detail  C: %d\n Module AB: %d\n Widget   : %d\n", counter_A, counter_B, counter_C, counter_AB, counter_W);

	return ALL_RIGHT;
}