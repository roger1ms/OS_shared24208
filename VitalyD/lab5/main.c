// gcc -g main.c -Wall -o main
// diff parent_maps.txt child_maps.txt
// kill
// ps aux | grep -E "|" | grep -v grep

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define ERR -1
#define CHILD 0
#define EXIT_CODE 5
#define BUF_SIZE 1024

int global = 123;

void print_vars(int* a, int* b) {
    printf("global = %d, addr = %p\n", *a, (void*)a);
    printf("local = %d, addr = %p\n", *b, (void*)b);
}

void print_status(int status) {
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        printf("\nThe child exited with code: %d\n", exit_code);
    }
    else if (WIFSIGNALED(status)) {
        int signal = WTERMSIG(status);
        printf("\nThe child process was terminated by signal: %d\n", signal);
    }
    else if (WCOREDUMP(status)) {
        printf("\nThe child produced a core dump\n");
    }
}

void print_my_maps(char* file) {
    FILE *fp = fopen("/proc/self/maps", "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }
    FILE *f = fopen(file, "w");
    if (f == NULL) {
        perror("fopen");
        fclose(fp);
        return;
    }
    
    char line[BUF_SIZE];
    while (fgets(line, sizeof(line), fp) != NULL) {
        fprintf(f, "%s", line);
    }
    
    fclose(f);
    fclose(fp);
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
        printf("\nChild:\n");
        pid_t parent_pid = getppid();
        pid_t my_pid = getpid();
        printf("- PID: %d\n", my_pid);
        printf("- Parent PID: %d\n", parent_pid);
        sleep(20);

        printf("- Old vars:\n");
        print_vars(&global, &local);
        global = 456;
        local = 7654;
        printf("- New vars:\n");
        print_vars(&global, &local);
        print_my_maps("child_maps.txt");

        //raise(SIGKILL);
        sleep(5);

        exit(EXIT_CODE);
    }
    else {
        sleep(21);
        printf("\n\nParent:\n");
        print_vars(&global, &local);
        print_my_maps("parent_maps.txt");

        sleep(10);

        int status;
        wait(&status);
        print_status(status);
    }
}

int main() {
    task1();
}
