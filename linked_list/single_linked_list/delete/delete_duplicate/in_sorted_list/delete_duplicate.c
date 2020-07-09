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

void delete_duplicate(struct node **head_ref) {
    struct node *current = *head_ref;
    struct node *dup_node; //用來記重複的node

    while(current->next != NULL) {
        if (current->value == current->next->value) { //Found duplicate
            dup_node = current->next;
            current->next = dup_node->next; //接到下下一個
            free(dup_node);
        } else {
            current = current->next; //Move forward
        }
    }
}

int main() {
    struct node *head = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)
    int delete_key = 0;

    //Make duplicate
    append(&head, 5);
    append(&head, 5);
    append(&head, 5);
    for (int i=5; i<10; i++)
        append(&head, i);

    //Make duplicate
    append(&head, 10);
    append(&head, 10);
    append(&head, 10);
    append(&head, 11);
    append(&head, 12);
    append(&head, 13);
    append(&head, 13);
    
    printf("Current sorted list is :\n");
    print_list(head);

    //Do delete duplicate
    delete_duplicate(&head);

    printf("\nAfter delete duplicate:\n");
    print_list(head);

    return 0;
}
