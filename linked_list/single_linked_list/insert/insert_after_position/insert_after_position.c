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

void insert(struct node **head_ref, int position, int new_value) {
    struct node *new_node = malloc(sizeof(struct node *));
    new_node->value = new_value;
    new_node->next = NULL;
    struct node *current = *head_ref;

    while(current->value != position) {
        current = current->next;
    }

    new_node->next = current->next;
    current->next = new_node;
}

int main() {
    struct node *head = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)
    //Do append
    for (int i=0; i<10; i++)
        append(&head, i);
    print_list(head);

    //Do insert
    insert(&head, 5, 100);
    print_list(head);

    return 0;
}
