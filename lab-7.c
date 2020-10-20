#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define INCORRECT_ARGS -3
#define ALLOCATOR_ERROR -4
#define ERROR_TREAD_CREATE -1
#define ERROR_TREAD_JOIN -2
#define ALL_RIGHT 0
#define COUNT_OF_TREADS 4
#define BUF_SIZE 256

void printTreadError(int errCode, char * comment){
        char *errorLine = strerror(errCode);
        fprintf(stderr,"%s: %s\n",comment, errorLine);
}


typedef struct data {
        double res;
        int id;
        int N;
        int ITER;

} data;


int checkArgs(int argc, char **argv) {
	
	if (argc < 3) {
        perror("Too few arguments");
        return INCORRECT_ARGS;
    }
		
		
	int N = atoi(argv[1]);
    int ITER = atoi(argv[2]);
    if (ITER <= 0) {
        perror("Incorrect number of iterations");
        return INCORRECT_ARGS;
    }
	
	 
	if (N < 1 || _SC_THREAD_THREADS_MAX < N) {
		perror("invalid number of threads");
		return INCORRECT_ARGS;
	}
	
	return ALL_RIGHT;
}



void *work(void *arg) {
	
        data *res = (data *)arg;
        int i = res->id;
		
        double a = 1; double b = 1;
        for (; i < res->ITER; i+=res->N) {
                a = (2*2*i + 1);
                b = (2*(2*i+1) + 1);
                res->res += (1/(a)) - (1/(b));
        }
        pthread_exit(NULL);
}


double calcPi(pthread_t *thread, data *results, int N, int ITER){
	int error;
	double PI = 0;
	
	thread = (pthread_t*)malloc(sizeof(pthread_t)*N);
        if (thread== NULL) {
                perror("couldn't allocate memory for threads");
                return ALLOCATOR_ERROR;
        }

        results = (data*)malloc(sizeof(data)*N);
        if (results == NULL) {
                perror("couldn't allocate memory for data struct");
                return ALLOCATOR_ERROR;
        }

       
	for (int i = 0; i < N; ++i) {
                results[i].res = 0;
                results[i].id = i;
                results[i].N = N;
                results[i].ITER = ITER;
                error = pthread_create(&(thread[i]), NULL, &work, results + i);
                if (error!=ALL_RIGHT) {
                        printTreadError(error,"couldn't create thread");
                        return ERROR_TREAD_CREATE;
                }
        }

        for (int i = 0; i < N; ++i) {
                error = pthread_join(thread[i], NULL);
                if (error != ALL_RIGHT) {
                        perror("couldn't join threads");
                        return ERROR_TREAD_JOIN;
                }
                PI += results[i].res;
        }
		PI *= 4;
		free(results);
		return PI;
}


int main(int argc, char **argv) {
        pthread_t *thread = NULL;
        data *results = NULL;
        int N = 0;
        int ITER = 0;
        int i = 0;
        int error;
		double PI = 0;
		
		error = checkArgs(argc, argv);
		if (error != ALL_RIGHT){
			return INCORRECT_ARGS;
		}
        N = atoi(argv[1]);
        ITER = atoi(argv[2]);

		PI = calcPi(thread,results,N,ITER);
        if(PI < ALL_RIGHT){
			return PI;
		}
        printf("PI = %.16lf\n",PI);
        pthread_exit(NULL);
}


