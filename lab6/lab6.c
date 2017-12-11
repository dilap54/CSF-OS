/*
Вариант 3:
Соединиться с сервером [Redis](http://redis.io/) и записать значение "тест" по ключу "ключ".
Проверить наличие значения с помощью родного консольного клиента `redis-cli`.
*/

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

char message[] = "set "; //Запрос, который отправится на redis сервер
char buf[64]; //Переменная, в которую запишется ответ от redis сервера

//Самая главная функция
int main(int argc, char *argv[])
{
    if ( argc != 3 ) //Если количество аргументов не равно 3
    {
        //Написать подсказку для пользователя
        printf( "usage: %s <key> <value>\n", argv[0] );
        exit(1);
    }
    int sock; //Сокет
    struct sockaddr_in addr; //Адрес redis сервера

    //Инициализация сокета
    //AF_INET для сетевого протокола IPv4
    //SOCK_STREAM (надёжная потокоориентированная служба (сервис) или потоковый сокет)
    //PROTOCOL = 0 (стандартное значение)
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)//Если сокет инициализировался с ошибкой
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET; //Указываем тип адреса
    addr.sin_port = htons(6379); //Указываем порт
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); //Указываем сам адрес (INADDR_LOOPBACK = 127.0.0.1)
    printf("Connecting to Redis...\n");
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)//Если соединение сокета с адресом не удалось
    {
        perror("connect");
        exit(2);
    }
    printf("Connected to Redis\n");
    strcat(message, argv[1]);//Копируем первый аргумент в переменную сообщения
    strcat(message, " \"");//Копируем '"' в переменную сообщения
    strcat(message, argv[2]);//Копируем второй аргумент в переменную сообщения
    strcat(message, "\"");//Копируем '"' в переменную сообщения
    strcat(message, "\r\n");//Копируем '\r\n' в переменную сообщения
    printf(message);
    send(sock, message, strlen(message), 0);//Отправляем  сообщение в сокет
    recv(sock, buf, 12, 0);//Получаем ответ (12 байт) от сокета и сохраняем в buf
    printf(buf);

    close(sock);//Закрываем сокет

    return 0;
}