#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/mman.h>
#include <limits.h>

#define ERROR -1
#define CHILD 0
#define BUFFER_SIZE 4096
#define BUFFER_LEN (int)(BUFFER_SIZE/sizeof(unsigned int))
#define LIMIT_VALUE UINT_MAX

int is_ready = 0;
pid_t other_pid = 0;
int sigint_received = 0;
void handle() {
    is_ready = 1;
}

void handle_sigint(int sig) {
    sigint_received = 1;
    pid_t my_pid = getpid();
    printf("process %d receive SIGINT\n", my_pid);
    if (other_pid != 0) {
        kill(other_pid, SIGTERM);
    }
    
    exit(0);
}

void task() {
    void* shared_memory = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
    }

    pid_t fork_res = fork();
    if (fork_res == ERROR) {
        perror("fork");
    }

    signal(SIGINT, handle_sigint);
    
    if (fork_res == CHILD) {
        pid_t parent_pid = getppid();
        other_pid = parent_pid;
        printf("Parent pid: %d, child pid: %d\n", parent_pid, getpid());
        unsigned int* int_ptr = (unsigned int*)shared_memory;
        signal(SIGUSR1, handle);

        for (unsigned int i = 0; i < LIMIT_VALUE; i++) {
            *(int_ptr + (i % BUFFER_LEN)) = i;
            if (i % BUFFER_LEN == BUFFER_LEN - 1) {
                is_ready = 0;
                kill(parent_pid, SIGUSR1);
                while (!is_ready && !sigint_received) { pause(); }
                if (sigint_received) break;
            }
        }
        exit(0);    
    }

    if (fork_res != CHILD) {
        other_pid = fork_res;
        unsigned int* int_ptr = (unsigned int*)shared_memory;
        unsigned int iteration_num = LIMIT_VALUE / BUFFER_LEN;
        int iteration_cnt = 0;
        int error = 0;
        int error_cnt = 0;

        signal(SIGUSR1, handle);
        
        while (iteration_cnt < iteration_num && !sigint_received) {
            while(!is_ready && !sigint_received) { pause(); }
            if (sigint_received) break;

            int need_to_read = BUFFER_LEN;
            if (iteration_cnt == iteration_num - 1 && LIMIT_VALUE % BUFFER_LEN != 0) {
                need_to_read = LIMIT_VALUE % BUFFER_LEN;
            }
            
            for (int i = 0; i < need_to_read; i++) {
                unsigned int value = *(int_ptr + i);
                unsigned int expected = i + iteration_cnt * BUFFER_LEN;
                if (value != expected) {
                    error += value;
                    error_cnt++;
                }
            }
            
            kill(fork_res, SIGUSR1);
            iteration_cnt++;
            is_ready = 0;
        }
        
        int status;
        waitpid(fork_res, &status, 0);
        if (WIFSIGNALED(status)) {
            printf("Writer terminated by signal %d\n", WTERMSIG(status));
        }
        printf("%d %d\n", error, error_cnt);
    }
}

int main() {
    task();
    return 0;
}