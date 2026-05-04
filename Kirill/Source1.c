#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>

#define SUCCESS 0
#define FAILURE -1
#define ERROR_VAL -1
#define DIR_MODE 0755
#define TRUE 1
#define FALSE 0
#define CHUNK_SIZE (1024 * 1024) 
#define PATH_BUFFER_SIZE 1024
#define NAME_BUFFER_SIZE 256


void reverse_string(char* str) {
    int i = 0;
    int j = strlen(str) - 1;

    while (i < j) {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}
void reverse_buffer(char* buffer, size_t n) {
    for (size_t i = 0; i < n / 2; i = i + 1) {
        char tmp = buffer[i];
        buffer[i] = buffer[n - 1 - i];
        buffer[n - 1 - i] = tmp;
    }
}
long get_file_size(FILE* f) {
    if (fseek(f, 0, SEEK_END) == ERROR_VAL) {
        perror("Œÿ»¡ ¿ fseek");
        return ERROR_VAL;
    }
    return ftell(f);
}


int copy_file_reversed(const char* src_path, const char* dest_path) {
    FILE* source_filename = fopen(src_path, "rb");
    if (source_filename == NULL) {
        perror("Œÿ»¡ ¿ Œ“ –€“»ﬂ SOURCE ‘¿…À¿");
        return FAILURE;
    }
    FILE* dest_filename = fopen(dest_path, "wb");
    if (dest_filename == NULL) {
        fclose(source_filename);
        perror("Œÿ»¡ ¿ Œ“ –€“»ﬂ DEST ‘¿…À¿");
        return FAILURE;
    }

    long file_size = get_file_size(source_filename);
    if (file_size == ERROR_VAL) {
        fclose(source_filename);
        fclose(dest_filename);
        return FAILURE;
    }

    char* buffer = (char*)malloc(CHUNK_SIZE);
    if (buffer == NULL) {
        perror("Œÿ»¡ ¿ ¬€ƒ≈À≈Õ»ﬂ œ¿Ãﬂ“»");
        return FAILURE;
    }
    long remaining_read_bytes = file_size;


    while (remaining_read_bytes > 0 && buffer != NULL) {

        long to_read;
        if (remaining_read_bytes < CHUNK_SIZE)
            to_read = remaining_read_bytes;
        else
            to_read = CHUNK_SIZE;

        if (fseek(source_filename, remaining_read_bytes - to_read, SEEK_SET) == ERROR_VAL) {
            perror("Œÿ»¡ ¿ ” ¿«¿“≈Àﬂ fseek");
            free(buffer);
            fclose(source_filename);
            fclose(dest_filename);
            return FAILURE;
        }

        size_t n_read = fread(buffer, 1, (size_t)to_read, source_filename);
        if (n_read < (size_t)to_read) {
            if (ferror(source_filename)) {
                perror("Œÿ»¡ ¿ ÔË ˜ÚÂÌËË Ù‡ÈÎ‡");
                free(buffer);
                fclose(source_filename);
                fclose(dest_filename);
                return FAILURE;
            }
        }

        reverse_buffer(buffer, n_read);

        size_t n_written = fwrite(buffer, 1, n_read, dest_filename);
        if (n_written < n_read) {
            perror("Œ¯Ë·Í‡ ÔË Á‡ÔËÒË ‚ Ù‡ÈÎ");
            free(buffer);
            fclose(source_filename);
            fclose(dest_filename);
            return FAILURE;
        }

        remaining_read_bytes = remaining_read_bytes - (long)n_read;
    }

    free(buffer);
    fclose(source_filename);
    fclose(dest_filename);
    return SUCCESS;
}



int is_regular_file(const char* path) {
    struct stat file_stat;

    if (stat(path, &file_stat) == ERROR_VAL) {
        return FALSE;
    }

    if (S_ISREG(file_stat.st_mode)) {
        return TRUE;//1
    }
    return FALSE;//0
}

int process_directory(const char* source_dir_name, const char* dest_dir_name) {
    DIR* dir = opendir(source_dir_name);
    struct dirent* entry;

    if (dir == NULL) {
        perror("Œÿ»¡ ¿ Œ“ –€“»ﬂ ƒ»–≈ “Œ–»»");
        return FAILURE;
    }

    while ((entry = readdir(dir)) != NULL) {
        char source_full_fname[PATH_BUFFER_SIZE];
        char dest_full_fname[PATH_BUFFER_SIZE];
        char reverced_name[NAME_BUFFER_SIZE];

        if (strcmp(entry->d_name, ".") == SUCCESS || strcmp(entry->d_name, "..") == SUCCESS)
            continue;

        sprintf(source_full_fname, "%s/%s", source_dir_name, entry->d_name);

        if (is_regular_file(source_full_fname) == TRUE) {
            strncpy(reverced_name, entry->d_name, NAME_BUFFER_SIZE);
            reverse_string(reverced_name);
            sprintf(dest_full_fname, "%s/%s", dest_dir_name, reverced_name);
            copy_file_reversed(source_full_fname, dest_full_fname);
        }
    }
    closedir(dir);
    return SUCCESS;
}


int prepare_and_process(int argc, char* argv[]) {
    char source_dir[PATH_BUFFER_SIZE];
    char dest_dir[PATH_BUFFER_SIZE];
    char source_name_rev[NAME_BUFFER_SIZE];

    strncpy(source_dir, argv[1], PATH_BUFFER_SIZE);

    if (argc == 3) {
        strncpy(source_name_rev, source_dir, NAME_BUFFER_SIZE);

        size_t len = strlen(source_name_rev);
        if (len > 0 && source_name_rev[len - 1] == '/') source_name_rev[len - 1] = '\0';

        reverse_string(source_name_rev);
        snprintf(dest_dir, PATH_BUFFER_SIZE, "%s/%s", argv[2], source_name_rev);
    }
    else {
        strncpy(dest_dir, source_dir, PATH_BUFFER_SIZE);
        reverse_string(dest_dir);
    }

    if (mkdir(dest_dir, DIR_MODE) == ERROR_VAL) {
        if (errno != EEXIST) {
            perror("Œÿ»¡ ¿ —Œ«ƒ¿Õ»ﬂ ƒ»–≈ “Œ–»»");
            return FAILURE;
        }
    }

    return process_directory(source_dir, dest_dir);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "ERROR: lack of args\n");
        return FAILURE;
    }
    else if (argc > 3) {
        fprintf(stderr, "ERROR: too much of args\n");
        return FAILURE;
    }

    if (prepare_and_process(argc, argv) == FAILURE) {
        fprintf(stderr, "Œÿ»¡ ¿ ‘”Õ ÷»» prepare_and_process");
        return FAILURE;
    }

    return EXIT_SUCCESS;
}

