#include <stdio.h>
#include <stdlib.h>

struct node {
   int value;
   struct node *next;
};

void print_list(struct node *head) {
    int i = 0;
    while(head != NULL) {
        printf("%d(%p)\n", head->value, head);
        head = head->next;
        i++;
    }
}

void append(struct node **head_ref, int new_value, int make_loop) {
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

    if (make_loop) {
        current->next = new_node;
        new_node->next = (*head_ref)->next->next->next;
        printf("%d(%p)\n", (new_node->next)->value, new_node->next);
    } else {
        current->next = new_node;
    }
}

int search_loop(struct node **head_ref) {
    struct node *fast_ptr = *head_ref; //current存在的目的是為了尋找位置
    struct node *slow_ptr = *head_ref; //current存在的目的是為了尋找位置
    int found = 0;

    if (*head_ref == NULL) {
        return -1;
    }

    //Start move
    printf("\nDo search:\n");
    while (fast_ptr != NULL && fast_ptr->next !=NULL) {
        printf("slow_ptr=%p, fast_ptr=%p\n", slow_ptr, fast_ptr);
        fast_ptr = fast_ptr->next->next; //move two steps
        slow_ptr = slow_ptr->next; //move one steps
        if (fast_ptr == slow_ptr) {
            printf("Found loop\n");
            printf("slow_ptr=%p, fast_ptr=%p, value is %d\n", slow_ptr, fast_ptr, fast_ptr->value);
            found = 1;
            break;
        }
    }

    if (!found)
       return 0;

    //兩者相遇的點一定是loop當中的點，那就從這個點開始，再loop一次，直到又遇到自己為止，就是這個loo的長度
    struct node *meet_ptr = slow_ptr; //current存在的目的是為了尋找位置
    int loop_len = 1;
    while (meet_ptr->next != slow_ptr) {
        meet_ptr = meet_ptr->next;
        loop_len++;
    }
    return loop_len;
}

int main() {
    struct node *head = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)
    //Do append
    for (int i=11; i<20; i++)
        append(&head, i, 0);
    print_list(head);

    //Make a loop
    append(&head, 15, 1);

    //Do search
    int ret = search_loop(&head);

    if (ret)
        printf("\nLoop found!!! The length of this loop is %d\n", ret);
    else
        printf("\nThere is no loop\n");

    return 0;
}
