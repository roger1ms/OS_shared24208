#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <linux/limits.h>
#include <libgen.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define ERROR -1
#define SUCCESS 0
#define REQUIRED_ARGC 3
#define BUFFER_SIZE 4096
#define ZERO_OFFSET 0

void reverse_string(const char* str, char* reverse) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        reverse[i] = str[len - i - 1];
    }
    reverse[len] = '\0';
}

void reverse_buffer(char* buffer, size_t len) {
    for (size_t i = 0; i < len / 2; i++) {
        char tmp = buffer[i];
        buffer[i] = buffer[len - 1 - i];
        buffer[len - 1 - i] = tmp;
    }
}

int write_bytes_to_file(int fd, const char* buffer, size_t bytes_to_write) {
    size_t bytes_written_total = 0;
    while (bytes_written_total < bytes_to_write) {
        ssize_t bytes_written = write(fd, buffer + bytes_written_total, bytes_to_write - bytes_written_total);
        if (bytes_written == ERROR) {
            return ERROR;
        }
        bytes_written_total += (size_t)bytes_written;
    }
    return SUCCESS;
}

int reverse_and_write_file_content(const char* source_file_path, const char* dest_file_path) {
    int source_fd = open(source_file_path, O_RDONLY);
    if (source_fd == ERROR) {
        perror("Open source file error");
        return ERROR;
    }

    int dest_fd = open(dest_file_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (dest_fd == ERROR) {
        perror("Open destination file error");
        close(source_fd);
        return ERROR;
    }

    off_t source_file_size = lseek(source_fd, ZERO_OFFSET, SEEK_END);
    if (source_file_size == ERROR) {
        perror("Get source file size error");
        close(dest_fd);
        close(source_fd);
        return ERROR;
    }

    char buffer[BUFFER_SIZE];
    off_t offset = source_file_size;
    while (offset > 0) {
        size_t bytes_to_read = (offset >= (off_t)sizeof(buffer)) ? sizeof(buffer) : (size_t)offset;
        offset -= bytes_to_read;

        off_t lseek_result = lseek(source_fd, offset, SEEK_SET);
        if (lseek_result == ERROR) {
            perror("lseek source file error");
            close(dest_fd);
            close(source_fd);
            return ERROR;
        }
        
        ssize_t bytes_read = read(source_fd, buffer, bytes_to_read);
        if (bytes_read == ERROR) {
            perror("Read source file error");
            close(dest_fd);
            close(source_fd);
            return ERROR;
        }

        reverse_buffer(buffer, bytes_read);
        int write_result = write_bytes_to_file(dest_fd, buffer, (size_t)bytes_read);
        if (write_result == ERROR) {
            perror("Write to destination file error");
            close(dest_fd);
            close(source_fd);
            return ERROR;
        }
    }

    close(dest_fd);
    close(source_fd);
    return SUCCESS;
}

int copy_regular_file_reversed(DIR* source_dir, const char* source_dir_path, const char* dest_dir_path) {
    struct dirent* entry;
    while ((entry = readdir(source_dir)) != NULL) {
        char source_file_path[PATH_MAX];
        char dest_file_path[PATH_MAX];
    
        int source_file_path_len = snprintf(source_file_path, PATH_MAX, "%s/%s", source_dir_path, entry->d_name);
        if (source_file_path_len >= PATH_MAX) {
            fprintf(stderr, "Error: name of source file is too long\n");
            return ERROR;
        }
        if (source_file_path_len == ERROR) {
            perror("Construct source file path error");
            return ERROR;
        }

        struct stat entry_stat;
        if (stat(source_file_path, &entry_stat) == ERROR) {
            perror("Get file status error");
            return ERROR;
        }
        if (!S_ISREG(entry_stat.st_mode)) {
            continue;
        }
        
        size_t entry_name_len = strlen(entry->d_name);
        char entry_reverse_name[entry_name_len+1];
        reverse_string(entry->d_name, entry_reverse_name);
        
        int dest_file_path_len = snprintf(dest_file_path, PATH_MAX, "%s/%s", dest_dir_path, entry_reverse_name);
        if (dest_file_path_len >= PATH_MAX) {
            fprintf(stderr, "Error: name of destination file is too long\n");
            return ERROR;
        }
        if (dest_file_path_len == ERROR) {
            perror("Construct destination file path error");
            return ERROR;
        }
        int reverse_write_result = reverse_and_write_file_content(source_file_path, dest_file_path);
        if (reverse_write_result == ERROR) { 
            return ERROR;
        }
    }

    return SUCCESS;
}

int reverse_catalog(char* source_dir_path, char* dest_dir_path) {
    size_t source_dir_path_len = strlen(source_dir_path);
    if (source_dir_path_len >= PATH_MAX) {
        fprintf(stderr, "Error: name of source directory is too long\n");
        return ERROR;
    }   

    DIR* source_dir = opendir(source_dir_path);
    if (source_dir == NULL) {
        perror("Open directory error");
        return ERROR;
    }
    
    if (dest_dir_path >= PATH_MAX) {
        fprintf(stderr, "Error: name of destination directory is too long\n");
        return ERROR;
    }

    int mkdir_result = mkdir(dest_dir_path, S_IRWXU);
    if (mkdir_result == ERROR && errno != EEXIST) {
        perror("Creating dest directory error");
    }
    char path_copy2[PATH_MAX]; 
    strncpy(path_copy2, source_dir_path, PATH_MAX);

    char* name_of_dir = basename(path_copy2);
    size_t name_len = strlen(name_of_dir);
    char dir_reverse_name[name_len+1];
    reverse_string(name_of_dir, dir_reverse_name);

    char reverse_dir_path[PATH_MAX];
    snprintf(reverse_dir_path, PATH_MAX, "%s/%s",dest_dir_path, dir_reverse_name);
    mkdir_result = mkdir(reverse_dir_path, S_IRWXU);
    if (mkdir_result == ERROR && errno != EEXIST) {
        perror("Creating inverted directory error");
        closedir(source_dir);
        return ERROR;
    }

    int copy_result = copy_regular_file_reversed(source_dir, source_dir_path, reverse_dir_path);
    if (copy_result == ERROR) {
        closedir(source_dir);
        return ERROR;
    }

    closedir(source_dir);
    return SUCCESS;
}

int main(int argc, char *argv[]) {
    if (argc != REQUIRED_ARGC) {
        fprintf(stderr, "Usage: %s <path_to_src_dir> <path_to_dest_dir>\n", argv[0]);
        return ERROR;
    }
    int reverse_result = reverse_catalog(argv[1], argv[2]);
    if (reverse_result == ERROR) {
        return ERROR;
    }
    return SUCCESS;
}