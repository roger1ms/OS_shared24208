#include <stdio.h>

void* local_var_addr() {
    int local;
    local = 5;
    printf("Local var address: %p\n", (void*)&local);
    return (void*)&local;
}

int main() {
    void* var_addr = local_var_addr();
    printf("Local var address: %p\n", var_addr);
}