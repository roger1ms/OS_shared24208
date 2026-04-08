#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define ERROR -1

int global_init = 42;
int global_uninit;
const int global_const = 100;

void first_task() {
    int local_init = 10;
    int local_uninit;
    static int static_init = 20;
    static int static_uninit;
    const int const_local = 30;
    char local_str[] = "Hello";

    printf("Function var\n");
    printf("local init:    %p\n", (void*)&local_init);
    printf("local uninit:  %p\n", (void*)&local_uninit);

    printf("Const int:     %p\n", (void*)&const_local);
    printf("local str:     %p %p\n", (void*)&local_str, (void*)&local_str[5]);

    printf("\nstatic init:   %p\n", (void*)&static_init);
    printf("static uninit: %p\n", (void*)&static_uninit);    

    printf("\nGlobal var\n");
    printf("Global initialized:     %p\n", (void*)&global_init);
    printf("Global non-initialized: %p\n", (void*)&global_uninit);
    printf("Global const:           %p\n", (void*)&global_const);

    char cmd[256];
    pid_t pid = getpid();
    snprintf(cmd, sizeof(cmd), "cat /proc/%d/maps > program_maps.txt", pid);
    int sys_op_res = system(cmd);
    if (sys_op_res == ERROR) {
        perror("system operation cat error");
    }
}

int main() {
    first_task();
    return 0;
}