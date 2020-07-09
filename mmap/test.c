#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
    int fd;
    void *ptr;
    char filename[128];
    sprintf(filename, "/proc/sla/vincent_mmap_test");
    fd = open(filename, O_RDONLY | O_CREAT, 00777); //只能讀這個mmap file，要從其他地方寫

    if (fd == -1) {
        printf("open mmap file failed:%s\n", strerror(errno));
        return -1;
    }   
    ptr = mmap(NULL, 1024, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    return 0;
}
