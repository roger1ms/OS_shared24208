//export TEST_VAR="Hello"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENV_VAR_NAME "TEST_VAR"
#define ERROR -1
#define OVERWRITE_TRUE 1

void change_env_var() {
    char* env_value = getenv(ENV_VAR_NAME);
    if (env_value == NULL) {
        fprintf(stderr, "Environment variable %s is not set\n", ENV_VAR_NAME);
        fprintf(stderr, "Set it before running the program:\n");
        fprintf(stderr, "  export %s=\"Hello from shell\"\n", ENV_VAR_NAME);
        exit(ERROR);
    }
    printf("Value of environment variable %s: \"%s\"\n", ENV_VAR_NAME, env_value);

    char new_value[] = "Goodbye";
    int ret = setenv(ENV_VAR_NAME, new_value, OVERWRITE_TRUE);
    if (ret == ERROR) {
        perror("setenv");
        exit(ERROR);
    }

    env_value = getenv(ENV_VAR_NAME);
    printf("New value of environment variable %s: \"%s\"\n", ENV_VAR_NAME, env_value);
}
int main() {
    change_env_var();
    return 0;
}