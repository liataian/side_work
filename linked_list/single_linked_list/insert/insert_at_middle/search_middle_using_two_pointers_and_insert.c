#include <stdio.h>
#include <stdlib.h>

struct node {
   int value;
   struct node *next;
};

void print_list(struct node *head) {
    int i = 0;
    while(head != NULL) {
        printf("%d(%d) ", head->value, i);
        head = head->next;
        i++;
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

struct node * search_middle(struct node **head_ref) {
    struct node *fast_ptr = *head_ref;
    struct node *slow_ptr = *head_ref;
    int len = 0;

    if (*head_ref == NULL) {
        return NULL;
    }

    //Start move
    while (fast_ptr != NULL && fast_ptr->next !=NULL) {
        fast_ptr = fast_ptr->next->next; //move two steps
        slow_ptr = slow_ptr->next; //move one steps
    }

    return slow_ptr;

}

void insert_at_middle(struct node **head_ref, struct node **mid, int new_value) {
    struct node *new_node = malloc(sizeof(struct node *));
    new_node->value = new_value;
    new_node->next = NULL;

    struct node *current = *head_ref;
    struct node *midd = *mid;

    while (current != midd) {
        current = current->next;
    }
    new_node->next = midd->next;
    midd->next = new_node;
}

int main() {
    struct node *head = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)
    struct node *middle = NULL;
    //Do append
    for (int i=11; i<20; i++)
        append(&head, i);
    print_list(head);

    //Do search
    middle = search_middle(&head);
    if (middle != NULL)
        printf("middle value is %d (at %p)\n", middle->value, middle);

    insert_at_middle(&head, &middle, 25);
    print_list(head);

    return 0;
}
