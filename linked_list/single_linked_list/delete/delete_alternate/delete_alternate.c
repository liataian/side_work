#include <stdio.h>
#include <stdlib.h>

struct node {
   int value;
   struct node *next;
};

void print_list(struct node *head) {
    int position = 0;
    while(head!=NULL) {
        printf("%d(%d) ", head->value, position);
        head = head->next;
        position++;
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

void delete_alternate(struct node **head_ref) {
    struct node *current = *head_ref;
    struct node *prev = *head_ref; //還需要額外一個pointer用來紀錄要free的node，否則會不知道怎free

    struct node *delete_node = (*head_ref)->next;

    if (*head_ref == NULL) {
        printf("There is no node...\n");
        return;
    }

    if ((*head_ref)->next == NULL) {
        printf("There is only one node...");
        return;
    }

    while (prev != NULL && current->next != NULL) {
        current = current->next;
        prev->next = current->next;
        prev = current;
        current = current->next;
        free(prev);
        prev = current;
    }
}

int main() {
    struct node *head = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)

    for (int i=1; i<16; i++)
        append(&head, i);
    printf("Current list is :\n");
    print_list(head);

    //Do delete
    delete_alternate(&head);

    printf("\nAfter delete alternate:\n");
    print_list(head);
    return 0;
}
