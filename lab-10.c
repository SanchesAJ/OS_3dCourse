#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <sched.h>
#include <unistd.h>
#define ALLOCATOR_ERROR -4
#define ERROR_TREAD_CREATE -1
#define ERROR_MUTEX_CREATE -5
#define ERROR_CONDITION_CREATE -6
#define ERROR_BARRIER_CREATE -7

#define ERROR_MUTEX_DESTROY -8
#define ERROR_MUTEX_ARG -8
#define ERROR_CONDITION_DESTROY -9
#define ERROR_BARRIER_DESTROY -10
#define ERROR_TREAD_JOIN -2
#define ALL_RIGHT 0
#define COUNT_OF_TREADS 4



pthread_mutex_t mt[3];

int locked0 = 0;
int locked1 = 0;

void printTreadError(int errCode, char * comment) {
    char *errorLine = strerror(errCode);
    fprintf(stderr, "%s: %s\n", comment, errorLine);
}


void *printText(void *arg) {
    int code;
    int id = (int)arg;
    int t = id;
	
	//Начальный захват мутекса с индексом потока 
    code = pthread_mutex_lock(&mt[id]);
    if (code != ALL_RIGHT)
        printTreadError(code, "mutex lock error");

	//Свитч отвечает за синхронизацию начала печати. Используется 
    switch (id) {
    case 0:
        locked0 = 1;
        break;
    case 1:
        locked1 = 1;
        while (!locked0) {
            sched_yield();
        }
        break;
    default:;
    }

    for (int i = 0; i < 9; ++i, ++t) {
		/*Алгоритм работает с 3 мутексами. Изначально 0-му потоку принадлежит муткс с номером 0, а потоку номер 1 - с номером 1. 
		Для печати строки, потоку нужно взять мутекс,мутекс, которым он не владеет на текущий момент. 
		Как только строка напечатана, поток освобождает мутекс, которым владел до того, как взял второй. Обеспечивается такой порядок действий переменной t
		Таким образом, мы используем 3 мутекса, на каждой итерации цикла for передавая один из них. 
		(задавая промежуточное состояние, отсутствие которого и приводит к дедлоку для 2 мутексов.) 
		*/
        code = pthread_mutex_lock(&mt[(t + 1) % 3]);
        if (code != ALL_RIGHT)
            printTreadError(code, "mutex lock error");


        printf("Thread %d: string %d\n", id, i + 1);

        code = pthread_mutex_unlock(&mt[t % 3]);
        if (code != ALL_RIGHT)
            printTreadError(code, "mutex unlock error");

    }
	
	code = pthread_mutex_unlock(&mt[id]);
	if (code != ALL_RIGHT)
            printTreadError(code, "mutex unlock error");
		
    pthread_exit(NULL);
}


int  initResorces( pthread_t *thread){
	pthread_mutexattr_t mutattr;

	int code;
	//Инициализация атрибута мьютекса
    code = pthread_mutexattr_init(&mutattr);
    if (code != ALL_RIGHT) {
        printTreadError(code, "mutex attr init error");
        return ERROR_MUTEX_ARG;
    }

	//Рекомендовано использовать PTHREAD_MUTEX_ERRORCHECK мутексы - используем
	//Этот тип мутекса позволяет обнаружить недопустимые действия с мутексом 
	//(повторная разблокировка, попытка разблокировать взятый другим потоком и т.д) 
    code = pthread_mutexattr_settype(&mutattr, PTHREAD_MUTEX_ERRORCHECK);
    if (code != ALL_RIGHT) {
        printTreadError(code, "mutex attr  set error");
        return ERROR_MUTEX_ARG;
    }

	//Инициализируем мутксы созданными атрибутами
    for (int i = 0; i < 3; ++i) {
        code = pthread_mutex_init(&mt[i], &mutattr);
        if (code != ALL_RIGHT) {
            printTreadError(code, "mutex init error");
            return ERROR_TREAD_CREATE;
        }
    }

	//Уничтожаем более не нужные атрибуты
    code = pthread_mutexattr_destroy(&mutattr);
    if (code != ALL_RIGHT) {
        printTreadError(code, "mutex attr destroy error");
        return ERROR_MUTEX_ARG;
    }
	
	return ALL_RIGHT;
}

int destroyResorces(){
	//Разрушаем не нужные мутксы.
    for (int i = 0; i < 3; ++i) {
        int code = pthread_mutex_destroy(&mt[i]);
        if (code != ALL_RIGHT) {
            printTreadError(code, "mutex destroy error");
            return ERROR_MUTEX_DESTROY;
        }
    }
	
	return ALL_RIGHT;
}

int main(int argC, char **argV) {
    int code;
    pthread_t thread;
    
	code = initResorces(thread);
	if (code != ALL_RIGHT) {
        printTreadError(code, "create init resorces");
        return code;
    }
	
	//Запускаем дочерний поток
	//Аргумент - номер запускаемого потока. pthread_self не гарантирует соблюдение номеров
    code = pthread_create(&thread, NULL, printText, (void *)1);
    if (code != ALL_RIGHT) {
        printTreadError(code, "create thread error");
        return ERROR_TREAD_CREATE;
    }
	//Холостой цикл ожидания, пока дочерний поток не будет готов к исполнению.  
    while (!locked1) {
		//Данная функция заставляет поток откзаться от выделенного процессорного времени,
		//заставляя его ждать, пока он снова не окажется в голове очереди планирования
		//Легальный холостой цикл
        sched_yield();
    }

	//Основная функция
	//Аргумент - номер запускаемого потока. pthread_self не гарантирует соблюдение номеров
    printText((void *)0);

	//Ожидание дочерней(в основном для отчистки)
    code = pthread_join(thread, NULL);
    if (code != ALL_RIGHT) {
        printTreadError(code, "thread join error");
        return ERROR_TREAD_JOIN;
    }

	code = destroyResorces();
	if (code != ALL_RIGHT) {
        printTreadError(code, "create destroy resorces");
        return code;
    }
	
    pthread_exit(NULL);
}
