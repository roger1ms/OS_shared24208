#include <stdio.h>
#include <stdlib.h>

#define SUCCESS 0

int global_init = 10;
int global_unitit;
const int global_const = 100;

int* get_local_var_address() {
    int new_local = 15;
    return &new_local;
}

void variables() {
    int local_var = 2;
    static int local_static_var = 3;
    const int const_local_var = 4;

    printf("Local address: %p\n", (void*)&local_var);
    printf("Local static address: %p\n", (void*)&local_static_var);
    printf("Const local address: %p\n", (void*)&const_local_var);
}

int main() {

    int* temp = get_local_var_address();
    printf("Func local var address: %p\n", (void*)temp);

    printf("global init address: %p\n", (void*)&global_init);
    printf("global not init address: %p\n", (void*)&global_unitit);
    printf("global const address: %p\n", (void*)&global_const);

    variables();

    return SUCCESS;
}
