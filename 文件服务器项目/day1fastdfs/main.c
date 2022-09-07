#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "fdfs_upload_file.h"

int main() {
    char fildID[1024] = {0};
    upload_file1("/etc/fdfs/client.conf", "main.c", fildID, sizeof(fildID));
    printf("Multiprocess upload_file1 fildID:%s\n", fildID);
    printf("=================================\n");
    upload_file2("/etc/fdfs/client.conf", "main.c", fildID);
    printf("Call API upload_file2 fildID:%s\n", fildID);
}