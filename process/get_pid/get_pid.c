#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main()
{
    pid_t p = getpid();
    printf("my pid is %d\n", p); 
}
