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

void selection_sort(int *arr, int len) {
    int min_idx = 0;
    for (int i=0; i<len-1; i++) {
        min_idx = i; //假設當前最小為i
        for (int j=i+1; j<len; j++) {
            if (arr[j] < arr[min_idx]) { //如果這一輪後面有比i小的，就把當前最小設為j，直到找出此輪最小者
                min_idx = j;
            }
        }
        swap(&arr[i], &arr[min_idx]); //找到最小者後交換，繼續下一輪
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

    printf("Before selection sort:\n");
    print_array(array, array_len);

    selection_sort(array, array_len); //&array != array, array == &array[0]

    printf("After selection sort:\n");
    print_array(array, array_len);

    return 0;
}
