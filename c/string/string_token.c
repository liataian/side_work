#include <stdio.h>
#include <string.h>

int main() {
        char str[] = "192.168.42.1";
        char *token;
        char *rest = str;
        while ((token = strtok_r(rest, ".", &rest)))
                printf("%s\n", token);
        return(0);
}
