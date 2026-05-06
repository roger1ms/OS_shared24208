#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 100
#define SUCCESS 0
#define ERROR_RET_VAL -1

int create_buffer() {
    int size = BUFF_SIZE;
    const char* phrase = "hello world";


    char* buff1 = (char*)malloc(size);
    if (buff1 == NULL) {
        perror("Error of allocation memory: buff1\n");
        return ERROR_RET_VAL;
    }
    strcpy(buff1, phrase);

    printf("Buffer1 phrase before free: %s\n", buff1);
    free(buff1);
    printf("Buffer1 phrase after free: %s\n", buff1);



    char* buff2 = (char*)malloc(size);
    if (buff2 == NULL) {
        perror("Error of allocation memory: buff2\n");
        return ERROR_RET_VAL;
    }
    strcpy(buff2, phrase);

    printf("Buffer2 phrase before free: %s\n", buff2);

    char* cursor_buff2 = buff2 + (size / 2);
    free(cursor_buff2);

    printf("Buffer2 phrase after free: %s\n", buff2);
    return SUCCESS;
}

int main() {
    if (create_buffer() == ERROR_RET_VAL) {
        fprintf(stderr, "Error in func: create_buffer()");
    }
}
