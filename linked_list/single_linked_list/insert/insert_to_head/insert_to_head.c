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

void insert_to_head(struct node **head_ref, int new_value) {
    struct node *new_node = malloc(sizeof(struct node *));
    new_node->value = new_value;
    new_node->next = NULL;

    if (*head_ref == NULL) {
        *head_ref = new_node;
        return;
    }
    
    new_node->next = *head_ref;
    *head_ref = new_node;
}

int main() {
    struct node *head = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)
    for (int i=10; i>0; i--)
        insert_to_head(&head, i);
    print_list(head);
    return 0;
}
