#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void randomize() {
    int i;
    time_t t;
    srand((unsigned) time(&t));
}

void print_array(int *arr, int len){
    for (int i=0; i<len; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

void swap(int *a, int *b) {
   int tmp = *a;
   *a = *b;
   *b = tmp;
}

void insertion_sort(int *arr, int len) {
    int prev, current;
    for (int round=0; round<len; round++) {
        current = round;
        prev = current-1;
        while(prev >= 0) {
            if (arr[current] < arr[prev]) {
                swap(&arr[current], &arr[prev]);
                current--;
                prev--;
            } else break;
        }
    }
}

int main() { 
    //int array[10] = {8, 9, 5, 4, 2, 1, 20 ,11, 7, 3};
    int num;
    printf("Enter amount of number:\n");
    scanf("%d", &num);
    randomize();
    int array[num];
    for (int i=0; i<num; i++) {
        array[i] = rand() % num;
    }
    int array_len = sizeof(array)/sizeof(array[0]); //C沒辦法知道傳入function的array大小，只能另外傳入

    printf("Before insertion sort:\n");
    print_array(array, array_len);

    insertion_sort(array, array_len); //&array != array, array == &array[0]

    printf("After insertion sort:\n");
    print_array(array, array_len);

    return 0;
}
