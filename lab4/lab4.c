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

//Инициализируем нужные MUTEX переменные
pthread_mutex_t elfdeer_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex для переменных с количеством эльфов/оленей у двери
pthread_mutex_t door_mutex = PTHREAD_MUTEX_INITIALIZER; //какой то костыльный mutex для того, чтобы эльфы не заходили раньше того, как выйдет другой эльф
pthread_mutex_t rig_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex, нужный для pthread_cond_wait, когда санта распрягает оленей

//Инициализируем нужные переменные состояний
pthread_cond_t knock_on_the_door = PTHREAD_COND_INITIALIZER; //для потоков, ожидающих стук в дверь (для санты)
pthread_cond_t santa_admite_elf = PTHREAD_COND_INITIALIZER; //для ожидающих, когда санта пригласит эльфа
pthread_cond_t santa_release_elf = PTHREAD_COND_INITIALIZER; //когда санта отпустит эльфа
pthread_cond_t santa_admite_deers = PTHREAD_COND_INITIALIZER; //когда санта пригласит оленя
pthread_cond_t santa_release_deers = PTHREAD_COND_INITIALIZER; //когда санта отпустит оленя

int elf_count = 0;//Количество эльфов у двери
int deer_count = 0;//Количество оленей у двери

//Функция для вывода текущего времени с миллисекундами
void printtime(){
    //Скопипасченный откуда то код
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

//Функция, которая пробуждает поток санты, если у двери достаточное количество зверей
void knock_santa(){
    pthread_mutex_lock(&elfdeer_mutex);//Блокируем elfdeer_mutex для работы с переменными elf_count и deer_count
    if (elf_count >= K){ //Если у двери достаточное количество эльфов
        printtime(); // Напечатать время
        printf("%d Эльфов стучатся к санте\n", elf_count);
        pthread_cond_signal(&knock_on_the_door);//Cтук в дверь к Санте
    }
    if (deer_count == N){ //если у двери собрались все олени
        printtime(); // Напечатать время
        printf("%d Оленей стучатся к санте\n", deer_count);
        pthread_cond_signal(&knock_on_the_door);//Cтук в дверь к Санте
    }
    pthread_mutex_unlock(&elfdeer_mutex); //Освобождаем elfdeer_mutex, чтобы другие потоки могли работать c elf_count и deer_count
}

//Самая главная функция, которая запускается после запуска программы.
int main(){ 
    printtime(); // Напечатать время
    printf("Программа Санта-Клаус запущена.\n");
    pthread_t santa_thread; //переменная для потока санты
    pthread_t elf_thread[M]; //массив потоков эльфов
    pthread_t deer_thread[N]; //массив потоков оленей

    //Объявление, что функции santa elf deer будут, просто объявлены ниже
    void *santa(); 
    void *elf();
    void *deer();

    srand(time(NULL));//Сделать рандом рандомным (хз, влияет ли это на что то)

    pthread_create(&santa_thread, NULL, santa, NULL);//Запуск потока санты

    for (int i=0; i<M; i++){//Запуск потоков эльфов
        //Создать поток эльфа
        //&elf_thread[i] - место, куда поток сохраняется
        //NULL - атрибуты потока (null - использовать стандартные)
        //elf - функция, которую вызовет поток
        //i - аргумент, с которым поток вызовет функцию elf. elf(i). Нужно, чтобы поток знал свой номер
        pthread_create(&elf_thread[i], NULL, elf, i); 
    }
   
    for (int i=0; i<N; i++){//Запуск потоков оленей
        pthread_create(&deer_thread[i], NULL, deer, i);
    }
    
    pthread_join(&santa_thread, NULL);//Ждать завершения потока санты (но он никогда не завершится)
}

//Фукнция, которую вызовет поток эльфа
void *elf(int id){
    //printf("Эльф %d родился\n", id);
    while(1){ //Бесконечно
        int random = rand() % (10*1000*1000); // рандом от 0 до 10000000 мкс
        //printf("%d", random);
        usleep(random); //Спать random микросекунд

        pthread_mutex_lock(&elfdeer_mutex);//Заблокировать elfdeer_mutex для работы с переменной elf_count
        elf_count++; //Увеличить количество эльфов у двери
        printtime(); // Напечатать время
        printf("Эльф %d подошел к двери. Ожидающих эльфов: %d. Ожидающих оленей: %d\n", id, elf_count, deer_count);
        pthread_mutex_unlock(&elfdeer_mutex); //Разблокировать elfdeer_mutex
        

        usleep(200); //Спать 200 микросекунд (не помню, зачем)

        knock_santa(); //Проверить, нужно ли стучаться к санте

        //Ожидание приглашения санты
        pthread_cond_wait(&santa_admite_elf,&door_mutex);//Блокировка потока по переменной santa_admite_elf. Ожидание, пока кто нибудь не вызовет pthread_cond_signal(&santa_admite_elf) (это будет санта)
        printtime(); // Напечатать время
        printf("Эльф %d вошел. Ожидающих эльфов: %d. Ожидающих оленей: %d\n", id, elf_count, deer_count);
        
        //Ожидание выпуска санты
        pthread_cond_wait(&santa_release_elf,&door_mutex);
        printtime(); // Напечатать время
        printf("Эльф %d вышел.\n",id);
        pthread_mutex_unlock(&door_mutex);
    }
}

//Фукнция, которую вызовет поток санты
void *santa(){
    //printf("Санта родился\n");
    while(1){ //Бесконечно
        printtime(); // Напечатать время
        printf("Санта спит\n");
        //Ожидание стука в дверь
        pthread_cond_wait(&knock_on_the_door,&elfdeer_mutex);//Заблокировать поток, по переменной knock_on_the_door, пока кто нибудь не вызовет pthread_cond_signal(&knock_on_the_door)
        while(deer_count == N || elf_count>0){//Санта работает, пока у двери стоит хоть один эльф, или стоят все олени
            if (deer_count == N){//Если все олени у двери
                deer_count = 0; //Обнулить счетчик оленей у двери (это безопасно, ведь elfdeer_mutex заблокирован ранее вызовом pthread_cond_wait(&knock_on_the_door,&elfdeer_mutex))
                pthread_mutex_unlock(&elfdeer_mutex);//Разблокировать elfdeer_mutex
                printtime(); // Напечатать время
                printf("Санта развозит игрушки. Ожидающих эльфов: %d. Ожидающих оленей: %d\n", elf_count, deer_count);
                //Сгенерировать время, которое будет спать (развозить игрушки) санта
                int random = rand() % (7*1000*1000)+2*1000*1000; //7000000мкс = 7 секунд. random получится в диапазоне от 2000000мкс до 9000000мкс
                usleep(random);//Спать (рандомно, от 2 до 9 секунд)
                pthread_cond_broadcast(&santa_release_deers);//Отпустить оленей ВСЕХ СРАЗУ (разблокировать все потоки, заблокированные по santa_release_deers)
            }
            
            pthread_mutex_lock(&elfdeer_mutex);//заблокировать elfdeer_mutex (если оленей не было у двери, то elfdeer_mutex и так заблокирован, но хуже не будет)
            while(elf_count>0){//Если у двери есть эльфы
    
                pthread_mutex_lock(&door_mutex);//В инструкциях в интернете написано, что нужно блокировать mutex перед тем, как разблокировать поток, заблокированный по состоянию и Этому mutex'у
                pthread_cond_signal(&santa_admite_elf);//Разблокировать ОДИН поток (какой-хз), заблокированный по santa_admite_elf (поток эльфа, который ждет у двери)
                pthread_mutex_unlock(&door_mutex);
    
                elf_count--; //Уменьшить количество эльфов у двери
                pthread_mutex_unlock(&elfdeer_mutex);
                
    
                sleep(T); //Спать T секунд
    
                pthread_mutex_lock(&door_mutex);
                pthread_cond_signal(&santa_release_elf);//Выпуск эльфа
                pthread_mutex_unlock(&door_mutex);
    
                usleep(200);//Дать время эльфу выйти (костыль)
    
                pthread_mutex_lock(&elfdeer_mutex);
            }
        }
    }
}

//Функция, которую вызовет поток оленя
void *deer(int id){
    //printf("Олень %d родился\n", id);
    while(1){ //Бесконечно
        int random = rand() % (6*1000*1000);
        usleep(random);//Гуляет

        //Пришел к санте
        pthread_mutex_lock(&elfdeer_mutex);
        deer_count++;
        printtime(); // Напечатать время
        printf("Олень %d подошел к двери. Ожидающих эльфов: %d. Ожидающих оленей: %d\n", id, elf_count, deer_count);
        pthread_mutex_unlock(&elfdeer_mutex);

        usleep(200);
        knock_santa();

        //pthread_cond_wait(&santa_admite_deers, &rig_mutex);
        
        //Ожидание распряжения
        pthread_cond_wait(&santa_release_deers, &rig_mutex);
        printtime(); // Напечатать время
        printf("Олень %d пошел гулять.\n", id);
        pthread_mutex_unlock(&rig_mutex);
    }
}