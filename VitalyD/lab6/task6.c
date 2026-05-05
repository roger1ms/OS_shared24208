// gcc -g task6.c -Wall -o main

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>

#define ERR -1
#define SUC 0
#define CHILD 0
#define TRUE 1
#define STOP 1
#define CONTINUE 0

struct timespec timeout = {0, 100000000};

int reader_wait(pid_t writer, sigset_t* set) {
    int sig, err;
    while (TRUE) {
        sig = sigtimedwait(set, NULL, &timeout);
        if (sig == ERR && errno == EAGAIN) {
            printf("%d\n", getppid());
            if (getppid() != writer) {
                printf("Writer died, exiting\n");
                return STOP;
            }
            continue;
        }
        else if (sig == SIGINT) {
            printf("\nReader was stopped by SIGINT\n");
            err = kill(writer, SIGUSR2);
            if (err == ERR) {
                perror("kill");
                return STOP;
            }
            return STOP;
        }
        else if (sig == SIGUSR2) {
            printf("\nReader was stopped by writer\n");
            return STOP;
        }
        return CONTINUE;
    }
}
void reader(void* addr, int size, sigset_t* set) {
    pid_t writer = getppid();
    int res = mprotect(addr, size, PROT_READ);
    if (res == ERR) {
        perror("mprotect (child)");
        return;
    }
    int first = 1;
    unsigned int number, last_number;
    while (TRUE) {
        int err = kill(writer, SIGUSR1);
        if (err == ERR) {
            perror("kill");
            return;
        }
        int stop = reader_wait(writer, set);
        if (stop == STOP) {
            break;
        }

        unsigned int* int_addr = (unsigned int*)addr;
        for (int i = 0; i < size / sizeof(int); i++) {
            number = int_addr[i];
            if (first == 1) {
                first = 0;
            }
            else if (last_number + 1 != number) {
                printf("Numbers %u and %u are not consistently\n", number, last_number);
            }
            last_number = number;
        }
    }
}

int writer_wait(pid_t reader, sigset_t* set) {
    int sig, err;
    while (TRUE) {
        sig = sigtimedwait(set, NULL, &timeout);
        if (sig == ERR && errno == EAGAIN) {
            int status;
            pid_t ret = waitpid(reader, &status, WNOHANG);
            if (ret == reader) {
                printf("Reader died, exiting\n");
                return STOP;
            } else if (ret == ERR) {
                perror("waitpid");
                return STOP;
            }
            continue;
        }
        else if (sig == SIGINT) {
            printf("\nWriter was stopped by SIGINT\n");
            err = kill(reader, SIGUSR2);
            if (err == ERR) {
                perror("kill");
            }
            return STOP;
        }
        else if (sig == SIGUSR2) {
            printf("\nReader was stopped by writer\n");
            return STOP;
        }
        return CONTINUE;
    }
}
void writer(void* addr, int size, int fork_res, sigset_t* set) {
    pid_t reader = fork_res;
    unsigned int number = 0;
    int err;
    while (TRUE) {
        int stop = writer_wait(reader, set);
        if (stop == STOP) {
            break;
        }

        unsigned int* int_addr = (unsigned int*)addr;
        for (int i = 0; i < size / sizeof(int); i++) {
            int_addr[i] = number;
            //printf("%d: %u\n", i, int_addr[i]);
            number++;
        }
        err = kill(reader, SIGUSR1);
        if (err == ERR) {
            perror("kill");
            return;
        }
    }
}

int set_sigset(sigset_t* set) {
    int err = sigemptyset(set);
    if (err == ERR) {
        perror("sigemptyset");
        return ERR;
    }
    err = sigaddset(set, SIGUSR1); 
        if (err == ERR) {
        perror("sigaddset");
        return ERR;
    }
    err = sigaddset(set, SIGUSR2);
        if (err == ERR) {
        perror("sigaddset");
        return ERR;
    }
    err = sigaddset(set, SIGINT);
    if (err == ERR) {
        perror("sigaddset");
        return ERR;
    }
    err = sigprocmask(SIG_BLOCK, set, NULL);
    if (err == ERR) {
        perror("sigprocmask");
        return ERR;
    }

    return SUC;
}
void task6() {
    sigset_t set;
    int err = set_sigset(&set);
    if (err == ERR) {
        return;
    }

    int size = sysconf(_SC_PAGE_SIZE);
    if (size == ERR) {
        perror("sysconf");
        return;
    }
    void* addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        return;
    }

    pid_t fork_res = fork();
    if (fork_res == ERR) {
        perror("fork");
        return;
    }

    if (fork_res == CHILD) {
        reader(addr, size, &set);
    }
    else {
        writer(addr, size, fork_res, &set);
    }

    int res = munmap(addr, size);
    if (res == ERR) {
        perror("munmap");
        return;
    }
}

int main() {
    task6();
    return SUC;
}
