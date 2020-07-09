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

void delete(struct node **head_ref, int delete_position) {
    struct node *current = *head_ref;
    struct node *prev; //還需要額外一個pointer用來紀錄要free的node，否則會不知道怎free

    if (*head_ref == NULL) {
        printf("There is no node...\n");
        return;
    }

    if (delete_position == 0) { //如果要刪除的直接就是head
        *head_ref = current->next;
        free(current);
        return;
    }

    while(current != NULL && delete_position--) { //前進delete_position個位置
        prev = current;
        current = current->next;
    }
    
    if (current == NULL) {
        printf("Can not find position\n");
        return; //找不到這個key
    }

    prev->next = current->next;
    free(current);
}

int main() {
    struct node *head = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)
    int position = 0;

    for (int i=10; i<15; i++)
        append(&head, i);
    printf("Current list is :\n");
    print_list(head);

    while (1) {
        //Enter a position to delete
        printf("Please enter a position to delete:\n");
        scanf("%d", &position);

        //Do delete
        delete(&head, position);

        printf("\nAfter delete:\n");
        print_list(head);
    }
    return 0;
}
