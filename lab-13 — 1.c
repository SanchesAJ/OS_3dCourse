#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <stdlib.h>

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

sem_t semaphore1;
sem_t semaphore2;

typedef struct Context {
	sem_t *sem_my;
	sem_t *sem_prt;
	int msg;
} Context;


void *childText(void *arg) {

	int error;
	Context *numTread = (Context*)arg;
	for (int i = 0; i < 10; i++) {
		error = sem_wait(numTread->sem_my);
		if (error != ALL_RIGHT) {
			printTreadError(error, "error semaphore wait");
			exit(ERROR_SEMAPHORE_POST);
		}

		printf("Thread %d : %d\n", numTread->msg, i + 1);

		error = sem_post(numTread->sem_prt);
		if (error != ALL_RIGHT) {
			printTreadError(error, "error semaphore post");
			exit(ERROR_SEMAPHORE_WAIT);
		}

	}

}



int init_sems() {
	int error;
	error = sem_init(&semaphore1, 0, 1);
	if (error != ALL_RIGHT) {
		printTreadError(error, "can't init semaphore 1");
		return ERROR_SEMAPHORE_INIT;
	}
	error = sem_init(&semaphore2, 0, 0);
	if (error != ALL_RIGHT) {
		printTreadError(error, "can't init semaphore 2");
		return ERROR_SEMAPHORE_INIT;
	}
	return ALL_RIGHT;
}

int destroy_sems() {
	int error;
	error = sem_destroy(&semaphore1);
	if (error != ALL_RIGHT) {
		printTreadError(error, "can't destroy semaphore 1");
		return ERROR_SEMAPHORE_DESTROY;
	}
	error = sem_destroy(&semaphore2);
	if (error != ALL_RIGHT) {
		printTreadError(error, "can't destroy semaphore 2");
		return ERROR_SEMAPHORE_DESTROY;
	}
	return ALL_RIGHT;
}

int initContext(Context *cntx, sem_t *one, sem_t *two, int mess) {

	cntx->sem_my = one;
	cntx->sem_prt = two;
	cntx->msg = mess;

	return ALL_RIGHT;
}

int main() {
	pthread_t thread;
	int error;

	error = init_sems();
	if (error != ALL_RIGHT) {
		return ERROR_SEMAPHORE_INIT;
	}

	Context cntx1;
	error = initContext(&cntx1, &semaphore1, &semaphore2, 1);

	Context cntx2;
	error = initContext(&cntx2, &semaphore2, &semaphore1, 2);



	error = pthread_create(&thread, NULL, childText, (void*)(&cntx1));
	if (error != ALL_RIGHT) {
		printTreadError(error, "create thread error");
		return ERROR_TREAD_CREATE;
	}
	childText((void*)(&cntx2));

	error = pthread_join(thread, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error, "join thread error");
		return ERROR_TREAD_JOIN;
	}

	error = destroy_sems();
	if (error != ALL_RIGHT) {
		return ERROR_SEMAPHORE_DESTROY;
	}

	pthread_exit(NULL);
}

