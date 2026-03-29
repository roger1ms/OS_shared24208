// gcc -g vitaly4.c -Wall -o main
// nm ./main

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024
#define SSCANF_SUCCESS 3

int global_init = 100;
int global_uninit;
const int global_const = 300;


void print_memory_maps() {
    FILE *fp = fopen("/proc/self/maps", "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }
    
    char line[BUF_SIZE];
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }
    
    fclose(fp);
}

void print_address_region_name(void* address) {
    FILE *fp = fopen("/proc/self/maps", "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }

    char line[BUF_SIZE];
    unsigned long start, end;
    unsigned long addr = (unsigned long)address;
    char perms[5];
    while (fgets(line, sizeof(line), fp)) {
        int res = sscanf(line, "%lx-%lx %4s", &start, &end, perms);
        if (res != SSCANF_SUCCESS) {
            fprintf(stderr, "Incorrect format of /proc/self/maps");
            fclose(fp);
            return;
        }
        
        if (addr < start || end < addr) {
            continue;
        }

        char *type = "UNKNOWN";
        if (strstr(line, "[heap]")) type = "HEAP";
        else if (strstr(line, "[stack]")) type = "STACK";
        else if (strstr(line, "[vdso]")) type = "VDSO";
        else if (strstr(line, "[vvar]")) type = "VVAR";
        else if (strstr(line, "[vvar_vclock]")) type = "VVAR_VCLOCK";
        else if (strstr(line, "[vsyscall]")) type = "VSYSCALL";
        else if (strstr(line, ".so")) type = "LIB";
        else if (strstr(line, "/")) type = "PROGRAM";
        
        printf("%s - %s\n", type, perms);
        break;
    }
    
    fclose(fp);
}

void print_variables_addresses() {
    int local1 = 10;
    int local2 = -20;
    char local3 = 'A';
    
    static int static_init = 50;
    static int static_uninit;
    static const int static_const = 7;
    
    const int local_const = 1827;
    const char* const_str = "Local constant string";
    
    printf("1. Локальные переменные:\n");
    printf("   int local1\t = %d,\t адрес: %p - ", local1, (void*)&local1);
    print_address_region_name((void*)&local1);
    printf("   int local2\t = %d,\t адрес: %p - ", local2, (void*)&local2);
    print_address_region_name((void*)&local2);
    printf("   char local3\t = %c,\t адрес: %p - ", local3, (void*)&local3);
    print_address_region_name((void*)&local3);

    printf("\n2. Статические переменные:\n");
    printf("   static int static_init\t = %d,\t адрес: %p - ", static_init, (void*)&static_init);
    print_address_region_name((void*)&static_init);
    printf("   static int static_uninit\t = %d,\t адрес: %p - ", static_uninit, (void*)&static_uninit);
    print_address_region_name((void*)&static_uninit);
    printf("   static const int static_const = %d,\t адрес: %p - ", static_const, (void*)&static_const);
    print_address_region_name((void*)&static_const);
    
    printf("\n3. Константы:\n");
    printf("   const int local_const\t = %d,\t\t адрес: %p - ", local_const, (void*)&local_const);
    print_address_region_name((void*)&local_const);
    printf("   const char* const_str\t = %p,\t адрес: %p - ", (void*)const_str, (void*)&const_str);
    print_address_region_name((void*)&const_str);
    printf("   \"%s\",\t\t\t\t адрес: %p - ", const_str, (void*)const_str);
    print_address_region_name((void*)const_str);

    printf("\n4. Глобальные инициализированные:\n");
    printf("   int global_init = %d, адрес: %p - ", global_init, (void*)&global_init);
    print_address_region_name((void*)&global_init);
    
    printf("\n5. Глобальные неинициализированные:\n");
    printf("   int global_uninit = %d, адрес: %p - ", global_uninit, (void*)&global_uninit);
    print_address_region_name((void*)&global_uninit);
    
    printf("\n6. Глобальные константы:\n");
    printf("   const int global_const = %d, адрес: %p - ", global_const, (void*)&global_const);
    print_address_region_name((void*)&global_const);

    printf("\n");
}

int main() {
    print_variables_addresses();
    print_memory_maps();
}