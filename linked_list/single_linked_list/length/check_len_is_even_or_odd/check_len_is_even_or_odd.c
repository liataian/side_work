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

int main() {
    struct node *head = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)
    //Do append
    for (int i=0; i<10; i++)
        append(&head, i);
    print_list(head);

    //Get length
    int len = get_length(&head);
    if (len%2 == 0)
        printf("Length of list is even\n");
    else
        printf("Length of list is odd\n");

    return 0;
}
