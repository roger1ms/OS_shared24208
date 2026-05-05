#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define ERROR -1
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define EXIT_CODE 5
#define CHILD 0
#define CMD_SIZE 256
int globalVar = 10;

void showVarAddress() {
    char cmd[CMD_SIZE];
    int localVar = 20;
    printf("Address of global variable: %p, value: %d\n", (void*)&globalVar, globalVar);
    printf("Address of local variable: %p, value: %d\n", (void*)&localVar, localVar);

    pid_t pid = getpid();
    printf("Process ID: %d\n", pid);

    pid_t forkResult = fork();
    if (forkResult == ERROR) {
        perror("Fork error");
        exit(EXIT_FAILURE);
    }
    if (forkResult == CHILD) {
        pid_t childPid = getpid();
        pid_t parentPid = getppid();
        printf("In child process\n");
        printf("    Child pid: %d, Parent pid: %d\n", childPid, parentPid);
        printf("    Address of global var: %p, value: %d\n", (void*)&globalVar, globalVar);
        printf("    Address of local var: %p, value: %d\n\n", (void*)&localVar, localVar);

        globalVar += 10;
        localVar += 10;
        
        printf("    Address, value of global var after modification: %p, %d\n", (void*)&globalVar, globalVar);
        printf("    Address, value of local var after modification: %p, %d\n", (void*)&localVar, localVar);
        snprintf(cmd, sizeof(cmd), "cat /proc/%d/maps > child_maps.txt", childPid);
        system(cmd);
        snprintf(cmd, sizeof(cmd), "ps %d > child_status.txt", childPid);
        system(cmd);
        exit(EXIT_CODE);
    }
    else if (forkResult > 0) {
        printf("\nIn parent process\n");
        printf("    Address of global var: %p, value: %d\n", (void*)&globalVar, globalVar);
        printf("    Address of local var: %p, value: %d\n", (void*)&localVar, localVar);
        sleep(30);

        
        snprintf(cmd, sizeof(cmd), "ps %d > child_status_before_wait.txt", forkResult);
        system(cmd);

        int childReturnValue;
        wait(&childReturnValue);

        snprintf(cmd, sizeof(cmd), "ps %d > child_status_after_wait.txt", forkResult);
        system(cmd);

        if (WIFEXITED(childReturnValue)) {
            printf("    Child exited normally with code: %d\n", WEXITSTATUS(childReturnValue));
        } else if (WIFSIGNALED(childReturnValue)) {
            printf("    Child was terminated by signal: %d\n", WTERMSIG(childReturnValue));
        } else if (WIFSTOPPED(childReturnValue)) {
            printf("    Child was stopped by signal: %d\n", WSTOPSIG(childReturnValue));
        } else {
            printf("    Child process ended with unknown status\n");
        }
        snprintf(cmd, sizeof(cmd), "cat /proc/%d/maps > parent_maps.txt", pid);
        system(cmd);
        snprintf(cmd, sizeof(cmd), "ps %d > parent_status.txt", pid);
        system(cmd);
    }
}

int main() {
    showVarAddress();
    return 0;
}