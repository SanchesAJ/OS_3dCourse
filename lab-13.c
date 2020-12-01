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


void *childText(void *arg) {
	int error;
	for (int i = 0; i < 10; i++) {
		error = sem_wait(&semaphore2);
		if (error != ALL_RIGHT) {
			printTreadError(error, "error semaphore wait");
			exit(ERROR_SEMAPHORE_POST);
		}
		printf("Thread 1: string %d\n", i + 1);

		error = sem_post(&semaphore1);
		if (error != ALL_RIGHT) {
			printTreadError(error, "error semaphore post");
			exit(ERROR_SEMAPHORE_WAIT);
		}

	}

}

void parentText() {
	int error;
	for (int i = 0; i < 10; i++) {
		error = sem_wait(&semaphore1);
		if (error != ALL_RIGHT) {
			printTreadError(error, "error semaphore wait");
			exit(ERROR_SEMAPHORE_POST);
		}
		printf("Thread 0: string %d\n", i + 1);
		error = sem_post(&semaphore2);
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

int main() {
	pthread_t thread;
	int error;

	error = init_sems();
	if (error != ALL_RIGHT) {
		return ERROR_SEMAPHORE_INIT;
	}

	error = pthread_create(&thread, NULL, &childText, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error, "create thread error");
		return ERROR_TREAD_CREATE;
	}
	parentText();

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

