// gcc -g task8.c -Wall -o main
// export MY_VAR="1"
// echo $MY_VAR

#include <stdio.h>
#include <stdlib.h>

#define ERR -1


void task8() {
    char *val = getenv("MY_VAR");
    if (val == NULL) {
        fprintf(stderr, "Error: MY_VAR is not defined.\n");
        return;
    }

    printf("value: %s\n", val);

    int err = setenv("MY_VAR", "2", 1);
    if (err == ERR) {
        perror("Setenv error");
    }
    printf("new value: %s\n", getenv("MY_VAR"));
}

int main() {
    task8();
}
