#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

int pipefd[2];
// pipefd[0] read pipe end
// pipefd[1] write pipe end
pid_t pid;

static void handle_parent_sigint(int sig)
{
    (void)sig;
    if (pipefd[1] != -1 || pipefd[0] != -1)
    {
        close(pipefd[0]);
        close(pipefd[1]);
    }
    printf("[Fuzzer] Sent all payloads. Closing pipe (sending EOF).\n");
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

int main()
{
    if (pipe(pipefd) < 0)
    {
        perror("pipe");
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
        signal(SIGINT, SIG_IGN); // Ignore SIGINT in child process
        close(pipefd[1]);
        // child process only reads from pipe

        if (dup2(pipefd[0], STDIN_FILENO) < 0)
        {
            perror("dup2");
            exit(1);
        }
        // Redirect stdin to read from pipe

        close(pipefd[0]);
        // No longer needed

        const char *preload = "./desockplus/desockplus.so";
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
        
        close(pipefd[0]);
        // parent process only writes to pipe
        sleep(1);

        // --- Sending payloads ---
        int counter = 0;
        while (1)
        {   
            char payload[128];
            snprintf(payload, sizeof(payload), "Fuzzing payload %d\n", counter++);
            printf("[Fuzzer] Sent: %s", payload);
            write(pipefd[1], payload, strlen(payload));
            sleep(1);
        }
    }
}