// gcc -g read.c -Wall -o read
// ./read
// sudo -u test_user ./read
// chmod u+s ./read

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define ERR -1
#define SUC 0
#define BUF_SIZE 256

int read_file() {
    FILE *file = fopen("private_file.txt", "r");
    if (file == NULL) {
        perror("fopen");
        return ERR;
    }

    char line[BUF_SIZE];
    while (fgets(line, sizeof(line), file) != NULL) {
        printf("%s", line);
    }

    int err = fclose(file);
	if (err == ERR) {
		perror("fclose");
        return ERR;
	}

    return SUC;
}

int main() {
	uid_t real_id = getuid();
	uid_t effective_id = geteuid();

	printf("Real id: %d\nEffective id: %d\n\n", real_id, effective_id);

    int res = read_file();
	if (res == ERR) {
		return ERR;
	}

    return SUC;
}
