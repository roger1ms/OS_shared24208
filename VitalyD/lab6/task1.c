// gcc -g task1.c -Wall -o main

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define ERR -1
#define SUC 0
#define CHILD 0
#define TRUE 1

void reader(void* addr, int size) {
    int res = mprotect(addr, size, PROT_READ);
    if (res == ERR) {
        perror("mprotect (child)");
        return;
    }
    int first = 1;
    unsigned int number, last_number;
    while (TRUE) {
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
void writer(void* addr, int size, int fork_res) {
    unsigned int number = 0;
    while (TRUE) {
        unsigned int* int_addr = (unsigned int*)addr;
        for (int i = 0; i < size / sizeof(int); i++) {
            int_addr[i] = number;
            //printf("%d: %u\n", i, int_addr[i]);
            number++;
        }
    }
}

void task1() {
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
        reader(addr, size);
    }
    else {
        writer(addr, size, fork_res);
    }


    int res = munmap(addr, size);
    if (res == ERR) {
        perror("munmap");
        return;
    }
}

int main() {
    task1();
}
