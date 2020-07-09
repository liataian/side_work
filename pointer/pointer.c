#include <stdio.h>
#include <stdlib.h>

//array本身是不會變的，但是你指摽是什麼型態你就要指向對應的型態

//ptr1 is an array of 5 pointers point to int
void test1(){
  int *ptr1[5]; //ptr1是一個包含五個指向int的pointers
  for (int i=0; i<5; i++) {
      printf("ptr1[%d]=%p, ptr1+%d=%p\n", i, *(ptr1+i), i, ptr1+i);
  }
  for (int i=0; i<5; i++) { //一次8個bytes
      printf("&ptr1+%d=%p\n", i, (&ptr1)+i);
  }
}

//ptr2 is a pointer points to an array of 5 ints
void test2() {
    int (*ptr2)[5]; //ptr2是一個指向包含五個int的array的pointer
    int bb[5] = {11,22,33,44,55}; //bb可以想成是一個指向包含五個int的array的pointer
    for (int i=0; i<sizeof(bb)/sizeof(bb[0]); i++) {
        printf("bb[%d]=%d ", i, bb[i]);
    }
    ptr2 = &bb; //bb或&bb[0]是第一個元素的位址，而&bb在"此處"可想成是一個指向包含五個int的array的pointer
    printf("\n&bb[0]=%p\n", &bb[0]);
    printf("bb=%p, bb+1=%p (加4 bytes)\n", bb, bb+1);
    printf("&bb=%p, &bb+1=%p (加20 bytes)\n", &bb, (&bb)+1);
    printf("ptr2 stores %p\n", ptr2);
}

// ptr3 is a pointer points to an int
void test3() {
    int *ptr3; //ptr3是一個指向一個int的pointer
    int cc[5] = {11,22,33,44,55};

    for (int i=0; i<sizeof(cc)/sizeof(cc[0]); i++) {
        printf("cc[%d]=%d ", i, cc[i]);
    }

    printf("\n\n##### ptr3 = cc; #####\n");
    ptr3 = cc; //cc跟&cc[0]都代表一個int的位址
    printf("&cc[0]=%p, cc=%p, &cc=%p\n", &cc[0], cc, &cc);
    printf("ptr3 stores %p, *ptr3=%d, &ptr3=%p\n", ptr3, *ptr3, &ptr3);

    printf("\n##### int **ptr33 = &ptr3; #####\n");
    int **ptr33 = &ptr3;
    printf("&ptr33=%p, ptr33=%p, *ptr33=%p, **ptr33=%d\n", &ptr33, ptr33, *ptr33, **ptr33);
    **ptr33 = 88;
    printf("*ptr3=%d\n", *ptr3);

}

int main() {
    printf("test 1: \n");
    test1();
    printf("\ntest 2: \n");
    test2();
    printf("\ntest 3: \n");
    test3();
    printf("\n");
    return 0;

}
