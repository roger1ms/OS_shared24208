// gcc -g -Wall -Wextra task.c -o main

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <limits.h>

#define ERROR -1
#define CHILD 0
#define SUCCESS 0
#define TRUE 1

void task() {
    long page_size_long = sysconf(_SC_PAGE_SIZE);
    if (page_size_long == ERROR) {
        perror("sysconf");
        return;
    }

    int page_size = (int)page_size_long;
    int buffer_len = page_size / (int)sizeof(unsigned int);

    void* shared_memory = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        return;
    }

    pid_t fork_res = fork();
    if (fork_res == ERROR) {
        perror("fork");

        int err = munmap(shared_memory, page_size);
        if (err == ERROR) {
            perror("munmap after fork error");
        }

        return;
    }

    if (fork_res == CHILD) {
        unsigned int* int_ptr = (unsigned int*)shared_memory;
        unsigned int value = 0;

        while (TRUE) {
            for (int i = 0; i < buffer_len; i++) {
                int_ptr[i] = value;
                value++;
            }
        }

        int err = munmap(shared_memory, page_size);
        if (err == ERROR) {
            perror("munmap child");
        }

        exit(SUCCESS);
    }

    if (fork_res != CHILD) {
        unsigned int* int_ptr = (unsigned int*)shared_memory;

        unsigned int expected = 0;

        while (TRUE) {
            for (int i = 0; i < buffer_len; i++) {
                unsigned int value = int_ptr[i];

                if (value != expected) {
                    printf("Sequence error: expected %u, got %u\n",
                        expected, value);
                }

                expected++;
            }
        }

        int err = munmap(shared_memory, page_size);
        if (err == ERROR) {
            perror("munmap parent");
        }
    }
}

int main() {
    task();
    return SUCCESS;
}