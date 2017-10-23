/*
Вариант 1:
Необходимо решить проблему "Санта Клаус" с использованием библиотеки PTHREAD с учетом следующих ограничений:
●	Санта все время спит, пока его не будет либо все его олени (число оленей — N), либо K из его M эльфов.
●	Если его будят олени, он впрягает их в сани, развозит игрушки в течение какого-то случайного времени, после чего распрягает их и отправляет погулять.
●	Каждый олень гуляет случайное время.
●	Если его будят эльфы, он проводит их по одному в свой кабинет, обсуждает с ними новые игрушки, а потом по одному отпускает.
●	Время обсуждения равно T. Каждый эльф после обсуждения с Сантой занимается своими делами случайное время.
●	Санта должен сначала поехать с оленями, если и олени и эльфы ждут его у дверей.
Результаты работы программы должны отображаться по мере их появления следующим образом. Вывод должен быть синхронизированным, т.е. последовательность строк в логе должна соответствовать последовательности операций, выполняемых программой.
...
12:01:10.353 Санта спит.
12:01:11.412 Эльф 5 подошел к двери. Ожидающих эльфов: 1. Ожидающих оленей: 0.
12:01:11.700 Эльф 1 подошел к двери. Ожидающих эльфов: 2. Ожидающих оленей: 0.
12:01:11.701 Олень 7 подошел к двери. Ожидающих эльфов: 2. Ожидающих оленей: 1.
12:01:11.910 Олень 8 подошел к двери. Ожидающих эльфов: 2. Ожидающих оленей: 2.
12:01:11.815 Олень 9 подошел к двери. Ожидающих эльфов: 2. Ожидающих оленей: 3.
12:01:14.810 Эльф 3 подошел к двери. 3 эльфа будят Санту.
12:01:14.951 Санта пропускает в кабинет эльфа 1.
12:01:15.115 Санта пропускает в кабинет эльфа 5.
12:01:15.222 Санта пропускает в кабинет эльфа 3.
12:01:17.815 Олень 3 подошел к двери. Ожидающих эльфов: 0. Ожидающих оленей: 4.
12:01:17.900 Санта с эльфами начинают совещание.
...

Параметры N, M, K и T задаются пользователем.
*/

#include <sys/time.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>

#define N 5 //Количество оленей
#define M 5 //Количество эльфов
#define K 4 //Количество эльфов у двери для пробуждения санты
#define T 1 //Время обсуждения каждого эльфа

pthread_mutex_t elf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t deer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elfdeer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t door_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rig_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t knock_on_the_door = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_admite_elf = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_release_elf = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_admite_deers = PTHREAD_COND_INITIALIZER;
pthread_cond_t santa_release_deers = PTHREAD_COND_INITIALIZER;

int elf_count = 0;
int deer_count = 0;

void printtime(){
    struct timeval tv;
    time_t nowtime;
    struct tm *nowtm;
    char tmbuf[64], buf[64];
    
    gettimeofday(&tv, NULL);
    nowtime = tv.tv_sec;
    nowtm = localtime(&nowtime);
    strftime(tmbuf, sizeof tmbuf, "%H:%M:%S", nowtm);
    printf("[%s.%06ld]:", tmbuf, tv.tv_usec);
}

void knock_santa(){
    pthread_mutex_lock(&elfdeer_mutex);
    if (elf_count >= K){
        printtime();
        printf("%d Эльфов стучатся к санте\n", elf_count);
        pthread_cond_signal(&knock_on_the_door);//Cтук в дверь к Санте
    }
    if (deer_count == N){
        printtime();
        printf("%d Оленей стучатся к санте\n", deer_count);
        pthread_cond_signal(&knock_on_the_door);//Cтук в дверь к Санте
    }
    pthread_mutex_unlock(&elfdeer_mutex);
}

int main(){
    printtime();
    printf("Программа Санта-Клаус запущена.\n");
    pthread_t santa_thread;
    pthread_t elf_thread[M];
    pthread_t deer_thread[N];

    void *santa();
    void *elf();
    void *deer();

    srand(time(NULL));

    pthread_create(&santa_thread, NULL, santa, NULL);//Запуск потока санты

    for (int i=0; i<M; i++){//Запуск потоков эльфов
        pthread_create(&elf_thread[i], NULL, elf, i);
    }
   
    for (int i=0; i<N; i++){//Запуск потоков оленей
        pthread_create(&deer_thread[i], NULL, deer, i);
    }

    pthread_join(&santa_thread, NULL);
}

void *elf(int id){
    //printf("Эльф %d родился\n", id);
    while(1){
        int random = rand() % (10*1000*1000);
        //printf("%d", random);
        usleep(random);

        pthread_mutex_lock(&elfdeer_mutex);
        elf_count++;
        printtime();
        printf("Эльф %d подошел к двери. Ожидающих эльфов: %d. Ожидающих оленей: %d\n", id, elf_count, deer_count);
        pthread_mutex_unlock(&elfdeer_mutex);
        

        usleep(200);

        knock_santa();

        pthread_cond_wait(&santa_admite_elf,&door_mutex);//Ожидание приглашения санты
        printtime();
        printf("Эльф %d вошел. Ожидающих эльфов: %d. Ожидающих оленей: %d\n", id, elf_count, deer_count);

        pthread_cond_wait(&santa_release_elf,&door_mutex);//Ожидание выпуска санты
        printtime();
        printf("Эльф %d вышел.\n",id);
        pthread_mutex_unlock(&door_mutex);
    }
}

void *santa(){
    //printf("Санта родился\n");
    while(1){
        printtime();
        printf("Санта спит\n");
        pthread_cond_wait(&knock_on_the_door,&elfdeer_mutex);//Ожидание стука в дверь
        while(deer_count == N || elf_count>0){//Санта работает, пока есть эльфы или есть все олени.
            if (deer_count == N){//Если все олени у двери
                deer_count = 0;
                pthread_mutex_unlock(&elfdeer_mutex);
                printtime();
                printf("Санта развозит игрушки. Ожидающих эльфов: %d. Ожидающих оленей: %d\n", elf_count, deer_count);
                int random = rand() % (7*1000*1000)+2000;
                usleep(random);
                pthread_cond_broadcast(&santa_release_deers);//Отпустить оленей
            }
    
            pthread_mutex_lock(&elfdeer_mutex);
            while(elf_count>0){//Если у двери есть эльфы
    
                pthread_mutex_lock(&door_mutex);
                pthread_cond_signal(&santa_admite_elf);//Приглашение эльфа
                pthread_mutex_unlock(&door_mutex);
    
                elf_count--;
                pthread_mutex_unlock(&elfdeer_mutex);
                
    
                sleep(T);
    
                pthread_mutex_lock(&door_mutex);
                pthread_cond_signal(&santa_release_elf);//Выпуск эльфа
                pthread_mutex_unlock(&door_mutex);
    
                usleep(200);//Дать время эльфу выйти
    
                pthread_mutex_lock(&elfdeer_mutex);
            }
        }
    }
}

void *deer(int id){
    //printf("Олень %d родился\n", id);
    while(1){
        int random = rand() % (6*1000*1000);
        usleep(random);//Гуляет

        //Пришел к санте
        pthread_mutex_lock(&elfdeer_mutex);
        deer_count++;
        printtime();
        printf("Олень %d подошел к двери. Ожидающих эльфов: %d. Ожидающих оленей: %d\n", id, elf_count, deer_count);
        pthread_mutex_unlock(&elfdeer_mutex);

        usleep(200);
        knock_santa();

        //pthread_cond_wait(&santa_admite_deers, &rig_mutex);
        
        //Ожидание распряжения
        pthread_cond_wait(&santa_release_deers, &rig_mutex);
        printtime();
        printf("Олень %d пошел гулять.\n", id);
        pthread_mutex_unlock(&rig_mutex);
    }
}