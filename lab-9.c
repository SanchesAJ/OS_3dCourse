#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define PHILO 5
#define DELAY 30000
#define FOOD 50

#define INCORRECT_ARGS -3
#define ALLOCATOR_ERROR -4
#define ERROR_TREAD_CREATE -1
#define ERROR_MUTEX_CREATE -5
#define ERROR_MUTEX_INIT -7
#define ERROR_CONDITION_CREATE -6


#define ERROR_MUTEX_DESTROY -8
#define ERROR_CONDITION_DESTROY -9


#define ERROR_TREAD_JOIN -2
#define ALL_RIGHT 0


pthread_mutex_t forks[PHILO];
pthread_t phils[PHILO];

pthread_mutex_t foodlock;

int sleep_seconds = 0;
pthread_mutex_t forks_taken_mutex;
int forks_taken = 0;

pthread_mutex_t waiter_mutex;
pthread_cond_t waiter_cond;

void printTreadError(int errCode, char * comment){
        char *errorLine = strerror(errCode);
        fprintf(stderr,"%s: %s\n",comment, errorLine);
}

/*Решение задачи  - введение в алгоритм философов официанта. Перед тем, как философ попытается взять вилки, 
он  должен получить разрешение у официанта. Официант знает количество свобрдных вилок, поэтому разрешить философу взять их,
 только  при наличии свободных вилок. Официант смотри количество свободных вилок, при малом их количестве - засыпает. 
 Соответственно просыпается он по бродкасту на cond.  Предикат - наличие 2 свободных вилок. 
*/

void waiter() {
	
	while (forks_taken >= PHILO - 1) {
		pthread_mutex_lock(&waiter_mutex);
		pthread_cond_wait(&waiter_cond, &waiter_mutex);
		pthread_mutex_unlock(&waiter_mutex);
	}
	
}

int food_on_table () {
	static int food = FOOD;
	int myfood;
	pthread_mutex_lock (&foodlock);
	if (food > 0) {
		food--;
	}
	myfood = food;
	pthread_mutex_unlock (&foodlock);
	return myfood;
}

void get_fork (int phil, int fork, char *hand) {
	//Увеличиваем количество взятых вилок
	pthread_mutex_lock (&forks_taken_mutex);
	++forks_taken;
	pthread_mutex_unlock (&forks_taken_mutex);
	
	//Берём вилку
	pthread_mutex_lock (&forks[fork]);
	printf ("Philosopher %d: got %s fork %d\n", phil, hand, fork);
}

void down_forks (int f1, int f2) {
	//Уменьшаем количество взятых вилок - отпускаем 2 штуки.

	pthread_mutex_lock (&forks_taken_mutex);
	--forks_taken; --forks_taken;
	pthread_mutex_unlock (&forks_taken_mutex);
	
	//Оповещаем официанта о освободившихся вилках
	pthread_mutex_lock(&waiter_mutex);
	pthread_cond_broadcast(&waiter_cond);
	pthread_mutex_unlock(&waiter_mutex);
	
	//Опускаем вилки
	pthread_mutex_unlock (&forks[f1]);
	pthread_mutex_unlock (&forks[f2]);
}

void * philosopher (void *num) {
	int id;
	int left_fork, right_fork, f;
	id = (int)num;
	printf ("Philosopher %d sitting down to dinner.\n", id);
	right_fork = id;
	left_fork = id + 1;
	
	//Заамыкаем круг вилок!
	if (left_fork == PHILO)
		left_fork = 0;
	
	//В начале проверяем количество оставшейся еды.
	while (f = food_on_table ()) {
		if (id == 1)
			sleep (sleep_seconds);
		
		//Философ берёт еду
		printf ("Philosopher %d: get dish %d.\n", id, f);
		//Философ спрашивает разрешения у официанта.
		waiter();
		get_fork (id, right_fork, "right");
		get_fork (id, left_fork, "left ");
		
		//Философ есть определённое количество времени
		printf ("Philosopher %d: eating.\n", id);
		usleep (DELAY * (FOOD - f + 1));
		
		//Опускаем вилки, оповещаем официанта о возврате вилок на место
		down_forks (left_fork, right_fork);
		printf ("Philosopher %d: put forks %d and %d\n", id, left_fork, right_fork);
	}
	printf ("Philosopher %d is done eating.\n", id);
	
	pthread_exit(NULL);
}




int main (int argn, char **argv) {
	int i;
	if (argn == 2)
		sleep_seconds = atoi (argv[1]);
	
	int error;
	//Два мутекса, закрывающие переменные количество взятых вилок и количество оставшейся еды.
	error = pthread_mutex_init (&foodlock, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't create mutex");
		return ERROR_MUTEX_INIT;
	}
	error = pthread_mutex_init (&forks_taken_mutex, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't create mutex");
		return ERROR_MUTEX_INIT;
	}
	
	
	//Условная переменная и мутекс, необходимые официанту
	error = pthread_mutex_init (&waiter_mutex, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't create mutex");
		return ERROR_MUTEX_INIT;
	}
	error = pthread_cond_init(&waiter_cond, NULL);
	if (error != ALL_RIGHT) {
		printTreadError(error, "couldn't create condition");
		return ERROR_CONDITION_CREATE;
	}
	
	
	//Инициализация мутексов имитирующих вилки
	for (i = 0; i < PHILO; i++){
		error = pthread_mutex_init (&forks[i], NULL);
		if (error != ALL_RIGHT) {
            printTreadError(error, "mutex init error");
            return ERROR_MUTEX_INIT;
        }
	}
	
	//Создание потоков.
	for (i = 0; i < PHILO; i++){
		error = pthread_create (&phils[i], NULL, philosopher, (void *)i);
		if (error != ALL_RIGHT) {
			printTreadError(error, "create thread error");
			return ERROR_TREAD_CREATE;
		}
	}
	
	//Ожидание завершенеия потоков.
	for (i = 0; i < PHILO; i++){
		error = pthread_join (phils[i], NULL);
		if (error != ALL_RIGHT) {
				printTreadError(error, "thread join error");
				return ERROR_TREAD_JOIN;
		}
	}
	
	pthread_exit(NULL);
}
