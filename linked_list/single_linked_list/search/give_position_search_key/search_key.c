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

int search(struct node **head_ref, int position) {
    struct node *current = *head_ref; //current存在的目的是為了尋找位置
    int pos = 0;

    if (*head_ref == NULL) {
        return -1;
    }

    if (position == 0) { //i.e head->value
        return current->value;
    }

    while(current != NULL) {
        if (pos == position) { //Get it
            return current->value;
        }
        current = current->next;
        pos++;
    }

    return -1; //Out of bound if reach here
}

int main() {
    struct node *head = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)
    //Do append
    for (int i=11; i<15; i++)
        append(&head, i);
    print_list(head);

    int search_position = 0;
    printf("Please give a position to search:\n");
    scanf("%d", &search_position);

    //Do search
    int ret = search(&head, search_position);

    if (ret >= 0)
        printf("\nKey at given position is %d\n", ret);
    else
        printf("\nCan not find this position\n");

    return 0;
}
