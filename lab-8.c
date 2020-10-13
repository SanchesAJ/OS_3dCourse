#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

int N = 0;
int alive;
int ITER = 0;
pthread_barrier_t barrier;
int inconst = 0;
pthread_mutex_t mutex;
pthread_cond_t cond;


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


void printTreadError(int errCode, char * comment){
        char *errorLine = strerror(errCode);
        fprintf(stderr,"%s: %s\n",comment, errorLine);
}


typedef struct data {
        double res;
        int id;
        long long int top;
} data;

data *results = NULL;
int flag = 1;
int n = 0;

void finalize(int signo) {
        flag = 0;
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
        while (flag) {
                for (i = 0; i < ITER; ++i) {
                        a = (2*2*j + 1);
                        b = (2*(2*j+1) + 1);
                        res->res += (1/(a)) - (1/(b));
                        j += N;
                }
                if (!flag) break;
                pthread_mutex_lock(&mutex);
                ++n;
                if (n < alive) {
                        pthread_cond_wait(&cond,&mutex);
                } else {
                        pthread_cond_broadcast(&cond);
                        n = 0;
                }
                pthread_mutex_unlock(&mutex);
                
        }
        pthread_mutex_lock(&mutex);
        --alive;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
        pthread_exit(NULL);
}


int main(int argc, char **argv) {
        pthread_t *thread = NULL;

        double PI = 0;
        int i = 0;
        if (argc < 2) {
                perror("Too few arguments");
                return 1;
        }
        int error;
        N = atoi(argv[1]);
        ITER = atoi(argv[2]);

        if (ITER <= 0) {
                perror("Incorrect number of iterations");
                return INCORRECT_ARGS;
        }

        alive = N;
        if (N <= 0) {
                perror("Incorrect number of threads");
                return 1;
        }



        thread = (pthread_t*)malloc(sizeof(pthread_t)*N);
        if (thread== NULL) {
                perror("couldn't allocate memory");
                return ALLOCATOR_ERROR;
        }

        results = (data*)malloc(sizeof(data)*N);
        if (results == NULL) {
                perror("couldn't allocate memory");
                return ALLOCATOR_ERROR;
        }

        error = pthread_barrier_init(&barrier, NULL, N);
        if (error!=ALL_RIGHT) {
                printTreadError(error,"couldn't setup barrier");
                return ERROR_BARRIER_CREATE;
        }
        error = pthread_mutex_init(&mutex, NULL);
        if (error!=ALL_RIGHT) {
                printTreadError(error,"couldn't create mutex");
                return ERROR_MUTEX_CREATE;
        }
        error = pthread_cond_init(&cond, NULL);
        if (error!=ALL_RIGHT) {
                printTreadError(error,"couldn't create condition");
                return ERROR_CONDITION_CREATE;
        }

        for (i = 0; i < N; ++i) {
                results[i].res = 0;
                results[i].id = i;
                error = pthread_create(&(thread[i]), NULL, &work, results + i);
                if (error!=ALL_RIGHT) {
                        printTreadError(error,"couldn't create thread");
                        return ERROR_TREAD_CREATE;
                }
        }
        signal(SIGINT, finalize);
        for (i = 0; i < N; ++i) {
                error = pthread_join(thread[i], NULL);
                if (error != ALL_RIGHT) {
                        perror("couldn't join threads");
                        return ERROR_TREAD_JOIN;
                }
                PI += results[i].res;
        }

        error = pthread_barrier_destroy(&barrier);
        if (error!=ALL_RIGHT) {
                printTreadError(error,"couldn't destroy barrier");
                return ERROR_BARRIER_DESTROY;
        }
        error = pthread_mutex_destroy(&mutex);
        if (error!=ALL_RIGHT) {
                printTreadError(error,"couldn't destroy mutex");
                return ERROR_MUTEX_DESTROY;
        }
        error = pthread_cond_destroy(&cond);
        if (error!=ALL_RIGHT) {
                printTreadError(error,"couldn't destroy condition");
                return ERROR_CONDITION_DESTROY;
        }

        free(results);
        PI *= 4;
        printf("PI = %.16lf\n",PI);
        fflush(0);
        pthread_exit(NULL);
}


