#include <stdio.h>

int B = 2;
void func(int *p) { p = &B; }
void func2(int *p) { *p = B;}

int main() {
    int A = 5566, C = 3;
    int *ptrA = &A; 
    printf("1 %d\n", *ptrA);
    func(ptrA);
    printf("2 %d\n", *ptrA);
    func2(ptrA);
    printf("3 %d\n", *ptrA);
    return 0;
}
