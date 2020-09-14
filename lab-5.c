#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define ERROR_TREAD_CREATE 1
#define ERROR_TREAD_CANCEL 2
#define ALL_RIGHT 0

void printTreadError(int errCode, char * comment){
	char *errorLine = strerror(errCode);
	fprintf(stderr,"%s: %s\n",comment, errorLine);
}

void *shut(void *arg) {
	printf("Thread cancelled\n");
}



void *work(void *arg) {
	//при помощи данных функций мы формируем стек обработчиков завершения потока. 
	//При завершении потока(принудительно или по умолчанию) все обработчики будут вызваны в порядке LIFO, 
	//только если они не были извлечены pthread_cleanup_pop.
	//Обработка ошибок не нужна, функции не могут вернуть код ошибки.
	pthread_cleanup_push(&shut,NULL)
	printf("I'm a child!\n");
	sleep(5);
	pthread_cleanup_pop(0);
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


