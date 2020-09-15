#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define ERROR_TREAD_CREATE 1
#define ALL_RIGHT 0
#define COUNT_OF_TREADS 4
#define COUNT_OF_STRING 4

const char *str[COUNT_OF_STRING] = {"String1", "String2", "String3", "String4"};

void printTreadError(int errCode, char * comment){
	char *errorLine = strerror(errCode);
	fprintf(stderr,"%s: %s\n",comment, errorLine);
}

void *work(void *arg) {
	
	//Используем pthread_self для получение идентификатора потока. Поскльку pthread_t - это фактически ручка(дескриптор), мы можем преобразовать его к числу.
	unsigned long int id = pthread_self();
	char **str = (char**)arg;
	printf("Thread %u: %s %s %s\n",id, str[id % COUNT_OF_STRING], str[(id+COUNT_OF_TREADS+1)%COUNT_OF_STRING],str[(COUNT_OF_TREADS-id)%COUNT_OF_STRING]);
	pthread_exit(NULL);

}

int main() {
	pthread_t thread[COUNT_OF_TREADS];
	int i = 0;

	int error;
	
	for (i = 0; i < COUNT_OF_TREADS; ++i){
		error = pthread_create(&thread[i], NULL, &work, str);
		if (error != ALL_RIGHT) {
			printTreadError(error,"Cannot create thread");
			return ERROR_TREAD_CREATE;
		}
	}
	

	
	pthread_exit(NULL);
}
