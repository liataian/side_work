#include <stdio.h>
#include <stdlib.h>

struct node {
   int value;
   struct node *next;
};

void print_list(struct node *head) {
    while(head!=NULL) {
        printf("%d ", head->value);
        head = head->next;
    }
    printf("\n");
}

void append(struct node **head_ref, int new_value) {
    struct node *new_node = malloc(sizeof(struct node *));
    new_node->value = new_value;
    new_node->next = NULL; //must be last node
    struct node *current = *head_ref; //current存在的目的是為了尋找位置

    if (*head_ref == NULL) {
        *head_ref = new_node;
        return;
    }   

    while(current->next != NULL) { //先找到最後一個node
        current = current->next;
    }   

    current->next = new_node;
}

int get_length(struct node **head_ref) {
    struct node *current = *head_ref; //current存在的目的是為了尋找位置
    int count = 1;

    if (*head_ref == NULL) {
        return 0;
    }

    while(current->next != NULL) {
        count++;
        current = current->next;
    }

    return count;
}

int sum_of_last_N_nodes(struct node **head_ref, int len, int num) {
    struct node *current = *head_ref;
    int start_sum_idx = len-num;
    int current_idx = 0;
    int sum = 0;
    if (*head_ref == NULL) return 0;
    if (len<=0) return 0;
    if (len==1) return current->value;

    while (current->next != NULL) { //只算到倒數第二個
        if (current_idx != start_sum_idx) {
            current_idx++;
        } else {
            sum += current->value;
        }
        current = current->next;
    }

    //加上最後一個
    sum += current->value;
    return sum;
}

int main() {
    struct node *head = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)
    //Do append
    for (int i=0; i<10; i++)
        append(&head, i);
    print_list(head);

    //Get length
    int len = get_length(&head);
    //printf("Length of list is %d\n", len);

    int n = 0;
    printf("Please specify how many nodes want to sum from last:\n");
    scanf("%d", &n);

    int res = sum_of_last_N_nodes(&head, len, n);
    printf("Sum of last %d nodes is %d\n", n, res);

    return 0;
}
