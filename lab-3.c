#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define ERROR_TREAD_CREATE 1
#define ERROR_TREAD_JOIN 2
#define ALL_RIGHT 0
#define COUNT_OF_TREADS 4
#define BUF_SIZE 256

void printTreadError(int errCode, char * comment){
	char *errorLine = strerror(errCode);
	fprintf(stderr,"%s: %s\n",comment, errorLine);
}

void *work(void *arg) {
	
	//Используем pthread_self для получение идентификатора потока. Поскльку pthread_t - это фактически ручка(дескриптор), мы можем преобразовать его к числу.
	unsigned long int id = pthread_self();
	 for (char **line = arg; *line != NULL; line++) {
                printf("Thread: %d: %s\n",id, *line);
        }
		
	pthread_exit(NULL);

}

int main() {
	pthread_t thread[COUNT_OF_TREADS];
	int i = 0;
	int error;
	char *str[COUNT_OF_TREADS][BUF_SIZE] = {{"String1","String2",NULL}, {"String2","String3",NULL}, {"String3","String4",NULL}, {"String4","String1",NULL}};

	
	for (i = 0; i < COUNT_OF_TREADS; ++i){
		error = pthread_create(&thread[i], NULL, &work, str[i]);
		if (error != ALL_RIGHT) {
			printTreadError(error,"Cannot create thread");
			return ERROR_TREAD_CREATE;
		}
	}
	
	for (i = 0; i < COUNT_OF_TREADS; ++i) {
            error = pthread_join(thread[i], NULL);
            if (error != ALL_RIGHT) {
                printTreadError(error,"Cannot join thread");
				return ERROR_TREAD_JOIN;
            }
    }
	

	
	pthread_exit(NULL);
}
