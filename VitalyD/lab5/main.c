// gcc -g main.c -Wall -o main
// diff parent_maps.txt child_maps.txt

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define ERR -1
#define CHILD 0
#define EXIT_CODE 5
#define BUF_SIZE 1024

int global = 123;

void print_memory_maps(char* path) {
    FILE *fp = fopen("/proc/self/maps", "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }
    FILE *dest = fopen(path, "w");
    if (dest == NULL) {
        fclose(fp);
        perror("fopen");
        return;
    }
    
    char line[BUF_SIZE];
    while (fgets(line, sizeof(line), fp) != NULL) {
        fprintf(dest, "%s", line);
    }
    
    fclose(fp);
    fclose(dest);
}

void print_vars(int* a, int* b) {
    printf("global = %d, addr = %p\n", *a, (void*)a);
    printf("local = %d, addr = %p\n", *b, (void*)b);
}

void task1() {
    int local = 4321;

    print_vars(&global, &local);

    pid_t pid = getpid();
    printf("PID: %d\n", pid); 

    pid_t fork_res = fork();
    if (fork_res == ERR) {
        perror("fork");
        return;
    }

    if (fork_res == CHILD) {
        pid_t parent_pid = getppid();
        pid_t my_pid = getpid();

        printf("\nChild:\n");
        printf("- PID: %d\n", my_pid);
        printf("- Parent PID: %d\n", parent_pid);
        printf("- Old vars:\n");
        print_vars(&global, &local);
        global = 456;
        local = 7654;
        printf("- New vars:\n");
        print_vars(&global, &local);
        print_memory_maps("child_maps.txt");

        exit(EXIT_CODE);
    }
    else {
        sleep(1);
        printf("\n\nParent:\n");
        print_vars(&global, &local);
        print_memory_maps("parent_maps.txt");
        sleep(30);

        int status;
        wait(&status);
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            printf("\nExit code: %d\n", exit_code);
        }
    }
}

int main() {
    task1();
}
