#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void buffer_task() {
    char *buffer = (char*)malloc(100 * sizeof(char));
    strcpy(buffer, "Hello, World!");
    printf("Buffer contents: \"%s\"\n", buffer);
    
    free(buffer);
    
    printf("Contents after free: \"%s\"\n", buffer);
    
    char *buffer2 = (char*)malloc(100 * sizeof(char));
    strcpy(buffer2, "Hello, World!");
    printf("New buffer contents: \"%s\"\n", buffer2);
    
    int offset = strlen(buffer2)/2;
    buffer2 += offset;
    free(buffer2);
    
    printf("Contents after free middle: \"%s\"\n", buffer2);
}

int main() {
    buffer_task();
    return 0;
}