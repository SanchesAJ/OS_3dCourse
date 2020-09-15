#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define ERROR_TREAD_CREATE 1
#define ERROR_TREAD_CANCEL 2
#define ALL_RIGHT 0
#define NOT_STOPED 1

void printTreadError(int errCode, char * comment){
	char *errorLine = strerror(errCode);
	fprintf(stderr,"%s: %s\n",comment, errorLine);
}


void *work(void *arg) {
	printf("I'm a child!\n");
	
	 while (NOT_STOPED){
		pthread_testcancel();
        printf("I'm a child in cycle!\n");
    }
	printf("I'm a child, i was't canceld!\n");
	pthread_exit(NULL);
}

int main() {
	pthread_t thread;
	int error;
	
	error = pthread_create(&thread, NULL, &work, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error,"Cannot create thread");
		return ERROR_TREAD_CREATE;
	}
	
	printf("Parent: sleep for 2 seconds.\n");
	sleep(2);
	error = pthread_cancel(thread);
	if (error != ALL_RIGHT) {
		printTreadError(error,"Cannot cancel thread");
		return ERROR_TREAD_CANCEL;
	}
	printf("Parent: cancelled thread.\n");


	
	pthread_exit(NULL);
}


