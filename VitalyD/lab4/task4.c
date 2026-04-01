// gcc -g task4.c -Wall -o main

#include <stdio.h>
#include <stdlib.h>


void* get_local_var_addr() {
    int local;
    local = 10;
    printf("%p\n", (void*)&local);
    return (void*)&local;
}

int main() {
    printf("%p\n", get_local_var_addr());
}
