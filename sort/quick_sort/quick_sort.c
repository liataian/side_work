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

//8, 9, 5, 4, 2, 1, 20 ,11, 7, 3
//精隨就是把每一輪的pivot歸到最終位置
int partition(int *arr, int left, int right) {
   int pivot = arr[left]; //永遠將第一個元素當作pivot
   int i = left; //從左邊開始，負責找比pivot大的
   int j = right; //從右邊開始，負責找比pivot小的

   //printf("\nStart partition, i=%d, j=%d\n", i, j);
   while (i < j) {
       while (arr[i] <= pivot) //如果i比pivot小，繼續向右走
           i++;
       while (arr[j] > pivot) //如果j比pivot大，繼續向左走
           j--;
       //i找到比pivot大的，j也找到比pivot小的
       if (i < j) {
           //printf("i=%d, j=%d, ", i, j);
           swap(&arr[i], &arr[j]); //兩者交換
           //printf("After swap: ");
           //print_array(arr, 10);
       }
   }
   //走到這代表j已經在i左邊了，j與pivot交換
   //printf("Do swap with pivot....\n");
   swap(&arr[j], &arr[left]);
   return j; //new index of pivot
}

void quick_sort(int *arr, int left, int right) { //use left to find bigger, use right to find smaller
    if (left >= right)
        return;

    //Do partition, 每次都要先找出新的切割點
    int cut_point = partition(arr, left, right);

    //printf("After swap with pivot: ");
    //print_array(arr, 10);

    //Do quick sort again, 從cut兩邊再分別做quick sort
    quick_sort(arr, left, cut_point-1);
    quick_sort(arr, cut_point+1, right);
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

    printf("Before quick sort:\n");
    print_array(array, array_len);

    //第一次的quick sort就是整個array (我們都假設第一個元素為pivot)
    quick_sort(array, 0, array_len-1); //&array != array, array == &array[0]

    printf("After quick sort:\n");
    print_array(array, array_len);

    return 0;
}
