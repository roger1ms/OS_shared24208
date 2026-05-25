// gcc -g create.c -Wall -o create
// ./create

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define ERR -1
#define SUC 0

int create_file() {
    int fd = open("private_file.txt", O_CREAT | O_WRONLY, S_IRUSR);

    if (fd == ERR) {
        perror("open");
        return ERR;
    }

    const char *text = "This is a secret message.\n";
    int err = write(fd, text, strlen(text));
	if (err == ERR) {
		perror("write");
        return ERR;
	}

    err = close(fd);
	if (err == ERR) {
		perror("close");
        return ERR;
	}

    return SUC;
}

int main() {
    int res = create_file();
	if (res == ERR) {
		return ERR;
	}

    return SUC;
}
