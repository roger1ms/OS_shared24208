// gcc -g task5.c -Wall -o main

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 100


void task5() {
    char* buf1 = (char*)malloc(SIZE*sizeof(char));
    strcpy(buf1, "Hello, world!");
    printf("%s\n", buf1);
    free(buf1);
    printf("%s\n", buf1);
    printf("\n");

    char* buf2 = (char*)malloc(SIZE*sizeof(char));
    strcpy(buf2, "Hello, world!");
    printf("%s\n", buf2);
    int shift = strlen(buf2)/2;
    buf2 += shift;
    printf("%s\n", buf2);
    free(buf2);
    printf("%s\n", buf2);
}

int main() {

    task5();
}
