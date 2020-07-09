#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct node {
   int value;
   struct node *next;
};

void print_list(struct node *head) {
    while(head!=NULL) {
        printf("%d->", head->value);
        head = head->next;
    }
    printf("NULL\n");
}

void append(struct node **head_ref, int new_value) {
    struct node *new_node = malloc(sizeof(struct node *));
    new_node->value = new_value;
    new_node->next = NULL; //must be last node
    struct node *current = *head_ref; //current存在的目的是為了尋找插入的位置

    if (*head_ref == NULL) {
        *head_ref = new_node;
        return;
    }

    while(current->next != NULL) { //先找到最後一個node
        current = current->next;
    }
    current->next = new_node;
}

int check_len(struct node *head_ref) {
    int len = 1;

    if (head_ref == NULL) return 0;
    if (head_ref->next == NULL) return 1;

    while(head_ref->next != NULL) {
        head_ref = head_ref->next;
        len++;
    }
    return len;
}

int calc_number(struct node *head_ref, int len) {
    struct node *current = head_ref;
    int num = 0;
    for (int i=len-1; i>=0; i--) {
        num += ((current->value) * pow(10.0, i)); //Run: gcc XXX.c -o XXX -lm
        current = current->next;
    }
    return num;
}

int subtract(struct node **head_ref1, struct node **head_ref2) {

#if 0 //method 1
    int ref1_len = check_len(*head_ref1);
    int ref2_len = check_len(*head_ref2);

    int num1 = calc_number(*head_ref1, ref1_len);
    int num2 = calc_number(*head_ref2, ref2_len);
    if (num1 > num2)
        printf("num1=%d, num2=%d, num1-num2=%d\n", num1, num2, num1-num2);
    else
        printf("num1=%d, num2=%d, num2-num1=%d\n", num1, num2, num2-num1);

#endif

#if 1 //method 2 very quick
    int num1 = 0, num2 = 0; 
    while (*head_ref1 || *head_ref2) {
        if (*head_ref1) { 
            num1 = num1*10 + (*head_ref1)->value; 
            (*head_ref1) = (*head_ref1)->next; 
        } 
        if (*head_ref2) {
            num2 = num2*10 + (*head_ref2)->value; 
            (*head_ref2) = (*head_ref2)->next; 
        } 
    }
    if (num1 > num2)
        printf("num1=%d, num2=%d, num1-num2=%d\n", num1, num2, num1-num2);
    else
        printf("num1=%d, num2=%d, num2-num1=%d\n", num1, num2, num2-num1);
#endif 
    return 0;
}

int main() {
    //First list
    struct node *head_1 = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)
    append(&head_1, 1);
    append(&head_1, 0);
    printf("First list:");
    print_list(head_1);

    //Second list
    struct node *head_2 = NULL;
    append(&head_2, 1);
    append(&head_2, 0);
    append(&head_2, 0);

    printf("Second list:");
    print_list(head_2);

    //Do multiply two lists
    subtract(&head_1, &head_2);

    return 0;
}
