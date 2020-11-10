#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

int N = 0;
int alive;
int ITER = 0;
int inconst = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;

#define ERROR_PI_CALC -1
#define INCORRECT_ARGS -3
#define ALLOCATOR_ERROR -4
#define ERROR_TREAD_CREATE -1
#define ERROR_MUTEX_CREATE -5
#define ERROR_CONDITION_CREATE -6
#define ERROR_BARRIER_CREATE -7

#define ERROR_MUTEX_DESTROY -8
#define ERROR_CONDITION_DESTROY -9
#define ERROR_BARRIER_DESTROY -10

#define ERROR_TREAD_JOIN -2
#define ALL_RIGHT 0
#define COUNT_OF_TREADS 4

int checkArgs(int argc, char **argv) {

	if (argc < 3) {
		perror("Too few arguments\n need: num of thread, count of iteration ");
		return INCORRECT_ARGS;
	}


	int N = atoi(argv[1]);
	int ITER = atoi(argv[2]);
	if (ITER <= 0) {
		perror("Incorrect number of iterations");
		fprintf(stderr, "Need 0<N<= %d", INT_MAX);
		return INCORRECT_ARGS;
	}


	if (N < 1 || _SC_THREAD_THREADS_MAX < N) {
		perror("invalid number of threads");
		fprintf(stderr, "Need 0<N<= %d", _SC_THREAD_THREADS_MAX);
		return INCORRECT_ARGS;
	}

	return ALL_RIGHT;
}


void printTreadError(int errCode, char * comment) {
	char *errorLine = strerror(errCode);
	fprintf(stderr, "%s: %s\n", comment, errorLine);
}


typedef struct data {
	double res;
	int id;
	long long int top;
} data;

int isInterrupt = 1;
int finishedThreads = 0;

void finalize(int signo) {
	isInterrupt = 0;
}



void *work(void *arg) {
	data *res = (data *)arg;
	long long int i = 0;
	long long int k = 0;
	long long int j = res->id;
	long long int max = 0;
	int br = 0;
	double a = 1; double b = 1;
	res->top = 0;
	int error;
	
	while (isInterrupt) {
		
		for (i = 0; i < ITER; ++i) {
			a = (2 * 2 * j + 1);
			b = (2 * (2 * j + 1) + 1);
			res->res += (1 / (a)) - (1 / (b));
			j += N;
		}
		
		if (!isInterrupt) break;

		error = pthread_mutex_lock(&mutex);

		if (error != ALL_RIGHT) {
			printTreadError(error, "couldn't lock mutex");
			exit(ERROR_PI_CALC);
		}
		++finishedThreads;
		while(finishedThreads < alive) {
			error = pthread_cond_wait(&cond, &mutex);
			if (error != ALL_RIGHT) {
				printTreadError(error, "couldn't block by cond");
				exit(ERROR_PI_CALC);
			}
		}
		//арьереы СЮДА!
		if(finishedThreads == alive) {
			error = pthread_cond_broadcast(&cond);
			if (error != ALL_RIGHT) {
				printTreadError(error, "couldn't make broadcast");
				exit(ERROR_PI_CALC);
			}
			finishedThreads = 0;
		}
		
		error = pthread_mutex_unlock(&mutex);

		if (error != ALL_RIGHT) {
			printTreadError(error, "couldn't unlock mutex");
			exit(ERROR_PI_CALC);
		}

	}

	error = pthread_mutex_lock(&mutex);

	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't lock mutex");
		exit(ERROR_PI_CALC);
	}
	--alive;

	error = pthread_cond_broadcast(&cond);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't make broadcast");
	}

	error = pthread_mutex_unlock(&mutex);

	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't unlock mutex");
		exit(ERROR_PI_CALC);
	}
	pthread_exit(NULL);
}

double calcPi(pthread_t *thread, data *results) {
	int error;
	double PI = 0;
	int i = 0;

	thread = (pthread_t*)malloc(sizeof(pthread_t)*N);
	if (thread == NULL) {
		perror("couldn't allocate memory");
		return ALLOCATOR_ERROR;
	}

	results = (data*)malloc(sizeof(data)*N);
	if (results == NULL) {
		free(thread);
		perror("couldn't allocate memory");
		return ALLOCATOR_ERROR;
	}


	error = pthread_mutex_init(&mutex, NULL);
	if (error != ALL_RIGHT) {
		free(results);
		free(thread);
		printTreadError(error, "couldn't create mutex");
		return ERROR_MUTEX_CREATE;
	}

	error = pthread_cond_init(&cond, NULL);
	if (error != ALL_RIGHT) {
		free(results);
		free(thread);
		printTreadError(error, "couldn't create condition");
		return ERROR_CONDITION_CREATE;
	}

	for (i = 0; i < N; ++i) {
		results[i].res = 0;
		results[i].id = i;
		error = pthread_create(&(thread[i]), NULL, &work, results + i);
		if (error != ALL_RIGHT) {
			printTreadError(error, "couldn't create thread");
			free(results);
			free(thread);
			return ERROR_TREAD_CREATE;
		}
	}

	//Всё готово к вычислениям. Устанавливаем обработчик сигнала
	signal(SIGINT, finalize);
	for (i = 0; i < N; ++i) {
		error = pthread_join(thread[i], NULL);
		if (error != ALL_RIGHT) {
			perror("couldn't join threads");
			free(results);
			free(thread);
			return ERROR_TREAD_JOIN;
		}
		PI += results[i].res;
	}
	free(results);
	free(thread);


	error = pthread_mutex_destroy(&mutex);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't destroy mutex");
		return ERROR_MUTEX_DESTROY;
	}
	error = pthread_cond_destroy(&cond);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't destroy condition");
		return ERROR_CONDITION_DESTROY;
	}

	PI *= 4;
	return PI;
}

int main(int argc, char **argv) {
	pthread_t *thread = NULL;
	data *results = NULL;
	double PI = 0;
	int error;

	error = checkArgs(argc, argv);
	if (error != ALL_RIGHT) {
		return INCORRECT_ARGS;
	}
	N = atoi(argv[1]);
	ITER = atoi(argv[2]);

	alive = N;
	PI = calcPi(thread, results);

	if (PI < ALL_RIGHT) {
		return ERROR_PI_CALC;
	}
	printf("PI = %.24lf\n", PI);

	pthread_exit(NULL);
}


