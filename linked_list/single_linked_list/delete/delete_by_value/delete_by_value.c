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

void delete(struct node **head_ref, int delete_value) {
    struct node *current = *head_ref;
    struct node *prev; //還需要額外一個pointer用來紀錄要free的node，否則會不知道怎free

#if 1 //Method 1
    if (current->value == delete_value) { //如果要刪除的直接就是head
        *head_ref = current->next;
        free(current);
        return;
    }

    while(current!=NULL && current->value != delete_value) {
        prev = current;
        current = current->next;
    }
    
    if (current == NULL) {
        printf("Can not find key %d\n", delete_value);
        return; //找不到這個key
    }

    prev->next = current->next;
    free(current);
#endif

#if 0 //Method 2
    while((current->next) != NULL && (current->next)->value != delete_value) { //先找到要被刪除的node的前一個node
        current = current->next;
    }

    if ((current->next) == NULL) {
        printf("Can not find key %d\n", delete_value);
        return; //找不到這個key
    }

    delete_node = current->next; //此時用delete_node先指向要被free的node

    current->next = (current->next)->next; //先用current串接好
    free(delete_node); //free掉
#endif
}

int main() {
    struct node *head = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)
    int delete_key = 0;

    for (int i=0; i<10; i++)
        append(&head, i);
    printf("Current list is :\n");
    print_list(head);

    //Enter a key to delete
    printf("Please enter a key to delete:\n");
    scanf("%d", &delete_key);

    //Do delete
    delete(&head, delete_key);

    printf("\nAfter delete:\n");
    print_list(head);
    return 0;
}
