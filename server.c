// server.c
// Usage: ./server <port>
// Listen only on localhost (127.0.0.1)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 1024

static int client_fd = -1;
static int listen_fd = -1;

static void handle_sigint(int sig)
{
    (void)sig;
    if (listen_fd != -1 || client_fd != -1)
    {
        close(client_fd);
        close(listen_fd);
    }
    fprintf(stderr, "Exiting.\n");
    exit(0);
}

void *recv_thread(void *arg)
{
    int fd = *(int *)arg;
    char buf[BUF_SIZE];
    for (;;)
    {
        int n = read(fd, buf, BUF_SIZE - 1);
        if (n <= 0)
        {
            printf("Client disconnected.\n");
            exit(0);
        }
        buf[n] = '\0';
        printf("Client: %s", buf);
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
        return 2;
    }

    signal(SIGINT, handle_sigint);

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535)
    {
        fprintf(stderr, "Invalid port.\n");
        return 1;
    }

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        perror("socket");
        return 1;
    }

    int enable = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, 16) < 0)
    {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    printf("Listening on 127.0.0.1:%d\n", port);

    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    int client_fd = accept(listen_fd, (struct sockaddr *)&client, &len);
    if (client_fd < 0)
    {
        perror("accept");
        return 1;
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client.sin_addr, ip, sizeof(ip));
    printf("Connection from %s:%d\n", ip, ntohs(client.sin_port));

    pthread_t tid_recv, tid_send;
    pthread_create(&tid_recv, NULL, recv_thread, &client_fd);
    pthread_create(&tid_send, NULL, send_thread, &client_fd);

    pthread_join(tid_recv, NULL);
    pthread_join(tid_send, NULL);

    close(client_fd);
    close(listen_fd);
    return 0;
}