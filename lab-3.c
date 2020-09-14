#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define ERROR_TREAD_CREATE 1
#define ALL_RIGHT 0


void printTreadError(int errCode, char * comment){
	char *errorLine = strerror(errCode);
	fprintf(stderr,"%s: %s\n",comment, errorLine);
}

void *work(void *arg) {
	//Используем pthread_self для получение идентификатора потока. Поскльку pthread_t - это фактически ручка(дескриптор), мы можем преобразовать его к числу.
	unsigned long int id = pthread_self();
	char **str = (char**)arg;
	sleep(2);
	printf("Thread %u: %s (%p)\n",id, str[id % 4], str[id%4]);
	pthread_exit(NULL);

}

int main() {
	pthread_t thread[4];
	char *str[4] = {"String1", "String2", "String3", "String4"};
	int i = 0;
	printf("%p\n%p\n",str, &i);

	int error;
	
	for (i = 0; i < 4; ++i){
		error = pthread_create(&thread[i], NULL, &work, str);
		if (error != ALL_RIGHT) {
			printTreadError(error,"Cannot create thread");
			return ERROR_TREAD_CREATE;
		}
	}
	

	
	pthread_exit(NULL);
}
