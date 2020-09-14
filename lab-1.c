#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

//Поток / нить - это наименьшая еденица планирования ("Наивное" определение)
//Поток  - нечто что создаёт иллюзию параллельного исполнения.(Определение с лекции)
#define ERROR_TREAD_CREATE 1
#define ALL_RIGHT 0

void printLines(char * value){
	for (int i = 0; i < 10; ++i) {
		printf("%s\n", value);
	}
}

void *work(void *arg) {
	printLines("I'm a child");

	pthread_exit(NULL);
}

int main() {
	pthread_t thread;
	int error;
	error = pthread_create(&thread, NULL, &work, NULL);
	if (error != ALL_RIGHT) {
		char *errorLine = strerror(error);
		fprintf(stderr,"Cannot create thread: %s\n",errorLine);
	
		return ERROR_TREAD_CREATE;
	}
	printLines("I'm a parent");

	
	pthread_exit(NULL);
}


