/*
Вариант 3:
Соединиться с сервером [Redis](http://redis.io/) и записать значение "тест" по ключу "ключ".
Проверить наличие значения с помощью родного консольного клиента `redis-cli`.
*/

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

char message[] = "set ";
char buf[64];

int main(int argc, char *argv[])
{
    if ( argc != 3 ) /* argc should be 2 for correct execution */
    {
        /* We print argv[0] assuming it is the program name */
        printf( "usage: %s <key> <value>\n", argv[0] );
        exit(1);
    }
    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(6379);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    printf("Connecting to Redis...\n");
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        exit(2);
    }
    printf("Connected to Redis\n");
    strcat(message, argv[1]);
    strcat(message, " \"");
    strcat(message, argv[2]);
    strcat(message, "\"");
    strcat(message, "\r\n");
    printf(message);
    send(sock, message, strlen(message), 0);
    recv(sock, buf, 12, 0);
    printf(buf);

    close(sock);

    return 0;
}