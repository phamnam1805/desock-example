// client.c
// Usage: ./client <port>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

#define BUF_SIZE 1024

void *recv_thread(void *arg)
{
    int fd = *(int *)arg;
    char buf[BUF_SIZE];
    for (;;)
    {
        int n = read(fd, buf, BUF_SIZE - 1);
        if (n <= 0)
        {
            printf("Server disconnected.\n");
            exit(0);
        }
        buf[n] = '\0';
        printf("Server: %s", buf);
    }
    return NULL;
}

void *send_thread(void *arg)
{
    int fd = *(int *)arg;
    char buf[BUF_SIZE];
    for (;;)
    {
        if (!fgets(buf, BUF_SIZE, stdin))
            break;
        write(fd, buf, strlen(buf));
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535)
    {
        fprintf(stderr, "Invalid port.\n");
        return 1;
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(port);

    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("connect");
        close(sock_fd);
        return 1;
    }

    printf("Connected to 127.0.0.1:%d\n", port);

    pthread_t tid_recv, tid_send;
    
    pthread_create(&tid_recv, NULL, recv_thread, &sock_fd);
    pthread_create(&tid_send, NULL, send_thread, &sock_fd);

    pthread_join(tid_recv, NULL);
    pthread_join(tid_send, NULL);

    close(sock_fd);
    return 0;
}