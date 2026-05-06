#include <stdio.h>
#include <stdlib.h>

#define SUCCESS 0
#define ERROR_RET_VAL -1

int env() {
    const char* var_name = "ENVIR_VAR";


    char* val = getenv(var_name);
    if (val == NULL) {
        fprintf(stderr, "Environment var %s not set\n", var_name);
        return ERROR_RET_VAL;
    }
    printf("Initial env value: %s\n", val);

    //setenv(var_name, "ChangedByProgram", 1); - linux
    _putenv_s(var_name, "ChangedByProgram");// - windows

    if (setenv(var_name, "ChangedByProgram", 1) != 0) {
        perror("setenv failed");
        return ERROR_RET_VAL;
    }

    val = getenv(var_name);
    printf("New env value: %s\n", val);
    return SUCCESS;

}

int main() {
    if (env() == ERROR_RET_VAL) {
        fprintf(stderr, "Error in func env()");
    }
}
