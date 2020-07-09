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

int search(struct node **head_ref, int value) {
    struct node *current = *head_ref; //current存在的目的是為了尋找位置
    int pos = 0;

    if (*head_ref == NULL) {
        return -1;
    }

    while(current != NULL) {
        if (current->value == value) //Get it
            return pos;
        current = current->next;
        pos++;
    }

    return -1; //Can not find
}

int main() {
    struct node *head = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)
    //Do append
    for (int i=10; i<15; i++)
        append(&head, i);
    print_list(head);

    int search_value = 0;
    printf("Please give a key to search its position:\n");
    scanf("%d", &search_value);

    //Get length
    int ret = search(&head, search_value);

    if (ret >= 0)
        printf("\nGiven key is %dth element\n", ret+1);
    else
        printf("\nCan not find this key\n");

    return 0;
}
