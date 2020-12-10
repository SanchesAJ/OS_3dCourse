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

void printTreadError(int errCode, char * comment) {
	char *errorLine = strerror(errCode);
	fprintf(stderr, "%s: %s\n", comment, errorLine);
}


volatile char is_stopped = 0;

sem_t sem_A, sem_B, sem_C, sem_mAB;

int counter_A = 0,
counter_B = 0,
counter_C = 0,
counter_AB = 0,
counter_W = 0;


void semPostWrapper(sem_t sem) {
	int error;
	error = sem_post(&sem);
	if (error != ALL_RIGHT) {
		printTreadError(error, "error semaphore post");
		exit(ERROR_SEMAPHORE_POST);
	}
}

void semWaitWrapper(sem_t sem) {
	int error;
	error = sem_wait(&sem);
	if (error != ALL_RIGHT) {
		printTreadError(error, "error semaphore post");
		exit(ERROR_SEMAPHORE_POST);
	}
}

void *createDetailA(void *arg) {
	int error;
	while (!is_stopped) {
		sleep(1);
		error = sem_post(&sem_A);
		if (error != ALL_RIGHT) {
			printTreadError(error, "error semaphore post");
			exit(ERROR_SEMAPHORE_POST);
		}
		printf("Created detail A\n");
		counter_A++;
	}
	pthread_exit(NULL);
}

void *createDetailB(void *arg) {
	int error;
	while (!is_stopped) {
		sleep(2);
		error = sem_post(&sem_B);
		if (error != ALL_RIGHT) {
			printTreadError(error, "error semaphore post");
			exit(ERROR_SEMAPHORE_POST);
		}
		printf("Created detail B\n");
		counter_B++;
	}
	pthread_exit(NULL);
}

void *createDetailC(void *arg) {
	int error;
	while (!is_stopped) {
		sleep(3);
		error = sem_post(&sem_C);
		if (error != ALL_RIGHT) {
			printTreadError(error, "error semaphore post");
			exit(ERROR_SEMAPHORE_POST);
		}
		printf("Created detail C\n");
		counter_C++;
	}
	pthread_exit(NULL);
}

void *createModuleAB(void *arg) {
	int error;
	while (!is_stopped) {
		error = sem_wait(&sem_A);
		if (error != ALL_RIGHT) {
			printTreadError(error, "error semaphore wait");
			exit(ERROR_SEMAPHORE_WAIT);
		}
		error = sem_wait(&sem_B);
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
	}
	pthread_exit(NULL);
}

void *createWidget(void *arg) {
	int error;
	while (!is_stopped) {
		error = sem_wait(&sem_C);
		if (error != ALL_RIGHT) {
			printTreadError(error, "error semaphore wait");
			exit(ERROR_SEMAPHORE_WAIT);
		}

		error = sem_wait(&sem_mAB);
		if (error != ALL_RIGHT) {
			printTreadError(error, "error semaphore wait");
			exit(ERROR_SEMAPHORE_WAIT);
		}
		printf("Created widget\n");
		counter_W++;
	}
	pthread_exit(NULL);
}


void sig_handler(int sig) {
	is_stopped = 1;
}

int init_sems() {
	int error;
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