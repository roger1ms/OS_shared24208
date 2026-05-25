// gcc -g -Wall -Wextra task.c -o main

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/mman.h>
#include <limits.h>
#include <errno.h>

#define ERROR -1
#define CHILD 0
#define SUCCESS 0
#define LIMIT_VALUE UINT_MAX

#define DATA_SIGNAL SIGUSR1
#define STOP_SIGNAL SIGUSR2

sigset_t set;

int prepare_signals() {
    int err = sigemptyset(&set);
    if (err == ERROR) {
        perror("sigemptyset");
        return ERROR;
    }

    err = sigaddset(&set, DATA_SIGNAL);
    if (err == ERROR) {
        perror("sigaddset SIGUSR1");
        return ERROR;
    }

    err = sigaddset(&set, STOP_SIGNAL);
    if (err == ERROR) {
        perror("sigaddset SIGUSR2");
        return ERROR;
    }

    err = sigaddset(&set, SIGINT);
    if (err == ERROR) {
        perror("sigaddset SIGINT");
        return ERROR;
    }

    err = sigprocmask(SIG_BLOCK, &set, NULL);
    if (err == ERROR) {
        perror("sigprocmask error");
        return ERROR;
    }

    return SUCCESS;
}

int wait_signal(int* received_signal) {
    int err = sigwait(&set, received_signal);
    if (err != SUCCESS) {
        perror("sigwait error");
        return ERROR;
    }

    return SUCCESS;
}

int send_signal(pid_t pid, int signal) {
    int err = kill(pid, signal);
    if (err == ERROR) {
        perror("kill");
        return ERROR;
    }

    return SUCCESS;
}

void task() {
    int err = prepare_signals();
    if (err == ERROR) {
        return;
    }

    long page_size_long = sysconf(_SC_PAGE_SIZE);
    if (page_size_long == ERROR) {
        perror("sysconf error");
        return;
    }

    int buffer_size = (int)page_size_long;
    int buffer_len = buffer_size / (int)sizeof(unsigned int);

    void* shared_memory = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap error");
        return;
    }

    pid_t fork_res = fork();
    if (fork_res == ERROR) {
        perror("fork error");

        err = munmap(shared_memory, buffer_size);
        if (err == ERROR) {
            perror("munmap after fork error");
        }

        return;
    }

    if (fork_res == CHILD) {
        pid_t parent_pid = getppid();

        printf("Parent pid: %d, child pid: %d\n", parent_pid, getpid());

        unsigned int* int_ptr = (unsigned int*)shared_memory;
        unsigned int i = 0;

        while (1) {
            *(int_ptr + (i % buffer_len)) = i;

            if (i % buffer_len == (unsigned int)(buffer_len - 1)) {
                err = send_signal(parent_pid, DATA_SIGNAL);
                if (err == ERROR) {
                    break;
                }

                int received_signal;
                err = wait_signal(&received_signal);
                if (err == ERROR) {
                    break;
                }

                if (received_signal == SIGINT) {
                    printf("Writer process %d received SIGINT\n", getpid());

                    err = send_signal(parent_pid, STOP_SIGNAL);
                    if (err == ERROR) {
                        break;
                    }

                    break;
                }

                if (received_signal == STOP_SIGNAL) {
                    printf("Writer process %d exits because reader stopped\n", getpid());
                    break;
                }

                if (received_signal != DATA_SIGNAL) {
                    printf("Writer process %d received unexpected signal %d\n",
                           getpid(), received_signal);
                }
            }

            i++;
            if (i == LIMIT_VALUE) {
                i = 0;
            }
        }

        err = munmap(shared_memory, buffer_size);
        if (err == ERROR) {
            perror("munmap child");
        }

        exit(SUCCESS);
    }

    unsigned int* int_ptr = (unsigned int*)shared_memory;
    unsigned int expected = 0;
    int error_cnt = 0;

    while (1) {
        int received_signal;
        err = wait_signal(&received_signal);
        if (err == ERROR) {
            break;
        }

        if (received_signal == SIGINT) {
            printf("Reader process %d received SIGINT\n", getpid());

            err = send_signal(fork_res, STOP_SIGNAL);
            if (err == ERROR) {
                break;
            }

            break;
        }

        if (received_signal == STOP_SIGNAL) {
            printf("Reader process %d exits because writer stopped\n", getpid());
            break;
        }

        if (received_signal != DATA_SIGNAL) {
            printf("Reader process %d received unexpected signal %d\n",
                    getpid(), received_signal);
            continue;
        }

        for (int j = 0; j < buffer_len; j++) {
            unsigned int value = *(int_ptr + j);

            if (value != expected) {
                printf("Sequence error: expected %u, got %u at index %d\n",
                        expected, value, j);
                error_cnt++;
            }

            expected++;
        }

        err = send_signal(fork_res, DATA_SIGNAL);
        if (err == ERROR) {
            break;
        }
    }

    int status;
    int wait_res = waitpid(fork_res, &status, 0);
    if (wait_res == ERROR) {
        perror("waitpid");
    }

    printf("Errors: %d\n", error_cnt);

    err = munmap(shared_memory, buffer_size);
    if (err == ERROR) {
        perror("munmap parent");
    }
}

int main() {
    task();
    return SUCCESS;
}