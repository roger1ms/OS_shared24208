#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/mman.h>
#include <limits.h>

#define ERROR -1
#define CHILD 0

void task() {
    void* shared_memory = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    pid_t fork_res = fork();
    if (fork_res == ERROR) {
        perror("fork");
    }

    if (shared_memory == MAP_FAILED) {
        perror("mmap");
    }

    if (fork_res == CHILD) {
        int* int_ptr = (int*)shared_memory;
        int max_i = 4096 / sizeof(int);
        for (int i = 0; i < INT_MAX; i++) {
            *(int_ptr + (i % max_i)) = i;
        }
        exit(0);    
    }

    if (fork_res != CHILD) {
        int* int_ptr = (int*)shared_memory;
        int max_i = 4096 / sizeof(int);
        int error = 0;
        int error_cnt = 0;
        for (int i = 0; i < 100; i++) {
            int value = *(int_ptr + (i%max_i));
            if (value != i) {
                error+=value;
                error_cnt++;
            }
            
        }   
        kill(fork_res, SIGKILL);
        printf("%d %d %d\n", error, error_cnt, *(int_ptr+3));
    }
}

int main() {
    task();
    return 0;
}