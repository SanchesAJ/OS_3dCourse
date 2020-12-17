#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#define ERROR_TREAD_CREATE -1
#define ERROR_TREAD_JOIN -2
#define ERROR_SEMAPHORE_WAIT -3
#define ERROR_SEMAPHORE_POST -4
#define ERROR_SEMAPHORE_INIT -5
#define ERROR_SEMAPHORE_DESTROY -6
#define ALL_RIGHT 0
#define ERROR_MUTEX -5
#define ERROR_MUTEX_INIT -7
#define ERROR_CONDITION_CREATE -6
#define ERROR_MUTEX_DESTROY -6

#define MAX_A 10
#define MAX_B 5
#define MAX_C 2
#define MAX_AB 3
#define MAX_WIDGET 10


void printTreadError(int errCode, char * comment) {
	char *errorLine = strerror(errCode);
	fprintf(stderr, "%s: %s\n", comment, errorLine);
}

void pthread_mutex_init_wrapper(pthread_mutex_t* mut) {
    int err_code = pthread_mutex_init (mut, NULL);
    if (err_code != ALL_RIGHT) {
        printTreadError(err_code, "error pthread_mutex_init");
		exit(ERROR_MUTEX_INIT);
    }
}

void pthread_cond_init_wrapper(pthread_cond_t* mut) {
    int err_code = pthread_cond_init(mut, NULL);
    if (err_code != ALL_RIGHT) {
        printTreadError(err_code, "error pthread_cond_init");
		exit(ERROR_CONDITION_CREATE);
    }
}

void pthread_sem_init_wrapper(sem_t* mut) {
    int err_code = sem_init(mut, 0,0);
    if (err_code != ALL_RIGHT) {
        printTreadError(err_code, "error sem_init");
		exit(ERROR_SEMAPHORE_INIT);
    }
}

void pthread_cond_signal_wrapper(pthread_cond_t* cond) {
    int err_code = pthread_cond_signal(cond);
    if (err_code != ALL_RIGHT) {
        printTreadError(err_code, "cannot desroy cond");
		exit(-1);
    }
}

void pthread_cond_wait_wrapper(pthread_cond_t* cond, pthread_mutex_t* mutex) {
    int err_code = pthread_cond_wait(cond, mutex);
    if (err_code != ALL_RIGHT) {
        printTreadError(err_code, "cannot desroy cond");
		exit(-1);
    }
}

void pthread_cond_destroy_wrapper(pthread_cond_t* cond) {
    int err_code = pthread_cond_destroy(cond);
    if (err_code != ALL_RIGHT) {
        printTreadError(err_code, "cannot desroy cond");
		exit(-1);
    }
}

void pthread_mutex_lock_wrapper(pthread_mutex_t* mutex) {
    int err_code = pthread_mutex_lock(mutex);
    if(err_code != ALL_RIGHT) {
        printTreadError(err_code, "cannot lock mutex");
		exit(ERROR_MUTEX);
    }
}

void pthread_mutex_unlock_wrapper(pthread_mutex_t* mutex) {
    int err_code = pthread_mutex_unlock(mutex);
    if(err_code != ALL_RIGHT) {
        printTreadError(err_code, "cannot unlock mutex");
		exit(ERROR_MUTEX);
    }
}

void sem_wait_wrapper(sem_t* sem) {
    int err_code = sem_wait(sem);
    if (err_code != ALL_RIGHT) {
        printTreadError(err_code, "error sem_wait");
		exit(ERROR_SEMAPHORE_WAIT);
    }
}

void sem_post_wrapper(sem_t* sem) {
    int err_code = sem_post(sem);
    if (err_code != ALL_RIGHT) {
        printTreadError(err_code, "error sem_POST");
		exit(ERROR_SEMAPHORE_POST);
    }
}

void sem_destroy_wrapper(sem_t* sem) {
    int err_code = sem_destroy(sem);
    if (err_code != ALL_RIGHT) {
        printTreadError(err_code, "error sem_destroy");
		exit(ERROR_SEMAPHORE_POST);
    }
}


void sem_getvalue_wrapper(sem_t* sem, int *A) {
    int err_code = sem_getvalue(sem, A);
    if (err_code != ALL_RIGHT) {
        printTreadError(err_code, "error sem_getvalue");
		exit(ERROR_SEMAPHORE_WAIT);
    }
}

volatile char is_stopped = 0;

sem_t sem_A, sem_B, sem_C, sem_mAB;
pthread_cond_t cond_A, cond_B, cond_C, cond_mAB;
pthread_mutex_t mut_A, mut_B, mut_C, mut_mAB;

int counter_A = 0,
counter_B = 0,
counter_C = 0,
counter_AB = 0,
counter_W = 0;

void *createDetailA(void *arg) {
	int error;

	while (!is_stopped) {
		int A;
		sem_getvalue_wrapper(&sem_A, &A);

		if(A <= MAX_A){
			sleep(1);
			sem_post_wrapper(&sem_A);
			printf("Created detail A\n");
			counter_A++;
		} else {
			printf("Cargo A is full. Wait\n");
			pthread_mutex_lock_wrapper(&mut_A);
			pthread_cond_wait_wrapper(&cond_A, &mut_A);
			pthread_mutex_unlock_wrapper(&mut_A);
		}
	}
	pthread_exit(NULL);
}

void *createDetailB(void *arg) {
	int error;
	while (!is_stopped) {
		int B;
		sem_getvalue_wrapper(&sem_B, &B);
		if(B <= MAX_B){
			sleep(2);
			sem_post_wrapper(&sem_B);
			printf("Created detail B\n");
			counter_B++;
		} else {
			printf("Cargo B is full. Wait\n");
			pthread_mutex_lock_wrapper(&mut_B);
			pthread_cond_wait_wrapper(&cond_B, &mut_B);
			pthread_mutex_unlock_wrapper(&mut_B);
		}
	}
	pthread_exit(NULL);
}

void *createDetailC(void *arg) {
	int error;
	while (!is_stopped) {
		int C;
		sem_getvalue_wrapper(&sem_C, &C);
		if(C <= MAX_C){
			sleep(3);
			sem_post_wrapper(&sem_C);
			printf("Created detail C\n");
			counter_C++;
			
		} else {
			printf("Cargo C is full. Wait\n");
			pthread_mutex_lock_wrapper(&mut_C);
			pthread_cond_wait_wrapper(&cond_C, &mut_C);
			pthread_mutex_unlock_wrapper(&mut_C);
		}
	}
	pthread_exit(NULL);
}

void *createModuleAB(void *arg) {
	int error;
	while (!is_stopped) {
		int AB;
		sem_getvalue_wrapper(&sem_mAB, &AB);
		if(AB <= MAX_AB){
			
			sem_wait_wrapper(&sem_A);
			pthread_cond_signal_wrapper(&cond_A);
			
			sem_wait_wrapper(&sem_B);
			pthread_cond_signal_wrapper(&cond_B);

			sem_post_wrapper(&sem_mAB);

			printf("Created module AB\n");
			counter_AB++;
		
		} else {
			printf("Cargo AB is full. Wait\n");
			pthread_mutex_lock_wrapper(&mut_mAB);
			pthread_cond_wait_wrapper(&cond_mAB, &mut_mAB);
			pthread_mutex_unlock_wrapper(&mut_mAB);
		}
	}
	pthread_exit(NULL);
}

void *createWidget(void *arg) {
	int error;
	while (!is_stopped) {
		
		if(counter_W != MAX_WIDGET){
			sem_wait_wrapper(&sem_C);
			pthread_cond_signal_wrapper(&cond_C);
			sem_wait_wrapper(&sem_mAB);
			pthread_cond_signal_wrapper(&cond_mAB);
			printf("Created widget\n");
			counter_W++;
		} else {
			printf("Cargo wigit is full. Finishing the program\n");
			is_stopped = 1;
			pthread_cond_signal_wrapper(&cond_A);
			pthread_cond_signal_wrapper(&cond_B);
			pthread_cond_signal_wrapper(&cond_C);
			pthread_cond_signal_wrapper(&cond_mAB);
			pthread_exit(NULL);
			
		}
	}
	pthread_exit(NULL);
}


void sig_handler(int sig) {
	is_stopped = 1;
}

int init_sems() {
	int error;
	
	pthread_mutex_init_wrapper(&mut_A);
	pthread_mutex_init_wrapper(&mut_B);
	pthread_mutex_init_wrapper(&mut_C);
	pthread_mutex_init_wrapper(&mut_mAB);
	
	pthread_cond_init_wrapper(&cond_A);
	pthread_cond_init_wrapper(&cond_B);
	pthread_cond_init_wrapper(&cond_C);
	pthread_cond_init_wrapper(&cond_mAB);
	
	pthread_sem_init_wrapper(&sem_A);
	pthread_sem_init_wrapper(&sem_B);
	pthread_sem_init_wrapper(&sem_C);
	pthread_sem_init_wrapper(&sem_mAB);
	
	return ALL_RIGHT;
}

int destroy_sems() {
	int error;
	sem_destroy_wrapper(&sem_A);
	sem_destroy_wrapper(&sem_B);
	sem_destroy_wrapper(&sem_C);
	sem_destroy_wrapper(&sem_mAB);
	
	error = pthread_mutex_destroy(&mut_A);
	if (error != ALL_RIGHT) {
		printTreadError(error, "mutex destroy error");
		return ERROR_MUTEX_DESTROY;
	}
	error = pthread_mutex_destroy(&mut_B);
	if (error != ALL_RIGHT) {
		printTreadError(error, "mutex destroy error");
		return ERROR_MUTEX_DESTROY;
	}
	error = pthread_mutex_destroy(&mut_C);
	if (error != ALL_RIGHT) {
		printTreadError(error, "mutex destroy error");
		return ERROR_MUTEX_DESTROY;
	}
	error = pthread_mutex_destroy(&mut_mAB);
	if (error != ALL_RIGHT) {
		printTreadError(error, "mutex destroy error");
		return ERROR_MUTEX_DESTROY;
	}
	
	pthread_cond_destroy_wrapper(&cond_A);
	pthread_cond_destroy_wrapper(&cond_B);
	pthread_cond_destroy_wrapper(&cond_C);
	pthread_cond_destroy_wrapper(&cond_mAB);
	return ALL_RIGHT;
}


int  work(){
	int error;
	pthread_t thread[5];
	error = pthread_create(&thread[0], NULL, createDetailA, NULL);
	if (ALL_RIGHT != error) {
		printTreadError(error, "creation thread(A) error");
		return ERROR_TREAD_CREATE;
	}
	error = pthread_create(&thread[1], NULL, createDetailB, NULL);
	if (ALL_RIGHT != error) {
		printTreadError(error, "creation thread(B) error");
		return ERROR_TREAD_CREATE;
	}
	error = pthread_create(&thread[2], NULL, createDetailC, NULL);
	if (ALL_RIGHT != error) {
		printTreadError(error, "creation thread(C) error");
		return ERROR_TREAD_CREATE;
	}
	error = pthread_create(&thread[3], NULL, createModuleAB, NULL);
	if (ALL_RIGHT != error) {
		printTreadError(error, "creation thread(AB) error");
		return ERROR_TREAD_CREATE;
	}
	error = pthread_create(&thread[4], NULL, createWidget, NULL);
	if (ALL_RIGHT != error) {
		printTreadError(error, "creation thread(W) error");
		return ERROR_TREAD_CREATE;
	}

	sigset(SIGINT, sig_handler);

	for (int i = 0; i < 5; ++i) {
		error = pthread_join(thread[i], NULL);
		if (ALL_RIGHT != error) {
			printTreadError(error, "join thread error");
		}
	}
	
}

int main(int argC, char **argV) {

	int error;

	error = init_sems();
	if (error != ALL_RIGHT) {
		return ERROR_SEMAPHORE_INIT;
	}

	error = work();
	if (error != ALL_RIGHT) {
		return error;
	}
	
	error = destroy_sems();
	if (error != ALL_RIGHT) {
		return ERROR_SEMAPHORE_DESTROY;
	}
	
	printf("#Created : \n Detail  A: %d\n Detail  B: %d\n Detail  C: %d\n Module AB: %d\n Widget   : %d\n", counter_A, counter_B, counter_C, counter_AB, counter_W);

	return ALL_RIGHT;
}