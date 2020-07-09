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

void do_merge(int *arr, int left, int mid, int right) {
    int n1 = mid-left+1;
    int n2 = right - mid;
    int right_arr[n1];
    int left_arr[n2];

    //原始array左半邊先複製到左array
    for (int i=0; i<n1; i++) {
           left_arr[i] = arr[left+i]; 
    }

    //原始array右半邊先複製到右array
    for (int i=0; i<n2; i++) {
           right_arr[i] = arr[mid+1+i]; 
    }

    int i = 0, j = 0;
    int k = left; //for 原始array的位置
    while (i<n1 && j<n2) {
        if (left_arr[i] <= right_arr[j]) { //左array值比右array值小
            arr[k] = left_arr[i];
            i++;
        } else { //右array值比左array值小
            arr[k] = right_arr[j];
            j++;
        }
        k++;
    }

    while (i<n1) { //左array還有剩，就直接複製
        arr[k] = left_arr[i];
        i++;
        k++;
    }

    while (j<n2) { //右array還有剩，就直接複製
        arr[k] = right_arr[j];
        j++;
        k++;
    }
}

void merge_sort(int *arr, int left, int right) {
    if (left >= right) { //已經切成一個element的case不用merge
        return;
    }

    //到這代表至少兩個element，開始動工
    int mid = (left+right)/2; //先找出中間點

    merge_sort(arr, left, mid);
    merge_sort(arr, mid+1, right);

    do_merge(arr, left, mid, right);
}

int main() {
    int num = 0;
    //int array[10] = {8, 9, 5, 4, 2, 1, 20 ,11, 7, 3};
    printf("Enter amount of numbers:\n");
    scanf("%d", &num);
    randomize();
    int array[num];
    for (int i=0; i<num; i++) {
        array[i] = rand() % num;
    }
    int array_len = sizeof(array)/sizeof(array[0]); //C沒辦法知道傳入function的array大小，只能另外傳入

    printf("Before merge sort:\n");
    print_array(array, array_len);

    merge_sort(array, 0, array_len-1); //&array != array, array == &array[0]

    printf("After merge sort:\n");
    print_array(array, array_len);
    return 0;
}
