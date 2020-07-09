#include <stdio.h>
#include <stdlib.h>

int gcd(int i, int j) {
    while(i != j){ 
        if(i>j) i-=j;
        else j -= i;
    }   
    return i;
}

int main() {
    int res = gcd(10, 13);
    printf("res=%d\n", res);
    return 0;
}

