#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

int input_pipe[2];
// parent writes → child stdin
// parent writes to input_pipe[1]
// whereas STDIN redirected to the same fd as input_pipe[0]
// -> child reads data which parent writes to input_pipe[1]
int output_pipe[2];
// child stdout → parent reads
// parent reads from output_pipe[0]
// child writes to STDOUT
// whereas STDOUT redirected to the same fd as output_pipe[1]
// -> parent reads data which child writes to output_pipe[1]
pid_t pid;

static void handle_parent_sigint(int sig)
{
    (void)sig;
    close(input_pipe[1]);
    close(output_pipe[0]);

    printf("\n[Fuzzer] Sent all payloads. Closing pipe (sending EOF).\n");
    int status;
    waitpid(pid, &status, 0);

    if (WIFSIGNALED(status))
    {
        printf("[Fuzzer] Client crashed with signal: %d\n", WTERMSIG(status));
    }
    else
    {
        printf("[Fuzzer] Client exited normally.\n");
    }
    fprintf(stderr, "Exiting.\n");
    exit(0);
}


void *recv_thread(void *arg)
{
    int fd = *(int *)arg;
    char recv_buf[1024];
    ssize_t n;
    while ((n = read(fd, recv_buf, sizeof(recv_buf) - 1)) > 0)
    {
        recv_buf[n] = '\0';
        printf("[Fuzzer] Received client output: %s", recv_buf);
    }
    printf("Client disconnected.\n");
    exit(0);
}

void *send_thread(void *arg)
{
    int fd = *(int *)arg;
    int counter = 0;
    while (1)
    {
        char payload[128];
        snprintf(payload, sizeof(payload), "Fuzzing payload %d\n", counter++);
        printf("[Fuzzer] Sent: %s", payload);
        write(fd, payload, strlen(payload));
        sleep(1);
    }
    return NULL;
}

int main()
{
    if (pipe(input_pipe) < 0)
    {
        perror("pipe input");
        return 1;
    }

    if (pipe(output_pipe) < 0)
    {
        perror("pipe output");
        return 1;
    }

    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return 1;
    }

    if (pid == 0)
    {
        signal(SIGINT, SIG_IGN);

        // Redirect STDIN to input_pipe[0]
        close(input_pipe[1]); // Child does not write to input_pipe[1]
        if (dup2(input_pipe[0], STDIN_FILENO) < 0)
        {
            perror("dup2 stdin");
            exit(1);
        }
        close(input_pipe[0]);

        // Redirect STDOUT to output_pipe[1]
        close(output_pipe[0]); // Child does not read from output_pipe[0]
        if (dup2(output_pipe[1], STDOUT_FILENO) < 0)
        {
            perror("dup2 stdout");
            exit(1);
        }
        close(output_pipe[1]);

        const char *preload = "./desock/desock.so";
        if (setenv("LD_PRELOAD", preload, 1) != 0)
        {
            perror("setenv");
            exit(1);
        }

        char *args[] = {
            "./bin/client",
            "5555",
            NULL};

        execvp(args[0], args);
        perror("execvp");
        exit(1);
    }
    else
    {
        signal(SIGINT, handle_parent_sigint);
        close(input_pipe[0]);  // Parent does not read from input_pipe[0]
        close(output_pipe[1]); // Parent does not write to output_pipe[1]


        // sleep(1);

        pthread_t tid_recv, tid_send;
        pthread_create(&tid_recv, NULL, recv_thread, &output_pipe[0]);
        pthread_create(&tid_send, NULL, send_thread, &input_pipe[1]);

        pthread_join(tid_recv, NULL);
        pthread_join(tid_send, NULL);
    }
}