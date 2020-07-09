#include <stdio.h>

int main() {
        int i;
        char *arr[50] = {"C","C++","Java","VBA"};
        char **ptr = arr;
        ptr[4] = "Sander";
        for ( i = 0; i < 10; i++ )
                printf("String %d : %s\n", i+1, ptr[i] );
        return 0;
}

