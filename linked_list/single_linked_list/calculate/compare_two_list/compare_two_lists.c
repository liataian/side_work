#include <stdio.h>
#include <stdlib.h>

struct node {
   int value;
   struct node *next;
};

void print_list(struct node *head) {
    while(head!=NULL) {
        printf("%c->", head->value);
        head = head->next;
    }
    printf("NULL\n");
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

int check_len(struct node *head_ref) {
    int len = 1;

    if (head_ref == NULL) return 0;
    if (head_ref->next == NULL) return 1;

    while(head_ref->next != NULL) {
        head_ref = head_ref->next;
        len++;
    }
    return len;
}

int compare(struct node **head_ref1, struct node **head_ref2) {

    int len_1 = check_len(*head_ref1);
    int len_2 = check_len(*head_ref2);

    if (len_1 > len_2) return 1;
    
    if (len_1 < len_2) return -1;

    while (*head_ref1 && *head_ref2) {
        if ((*head_ref1)->value == (*head_ref2)->value) { 
            (*head_ref1) = (*head_ref1)->next; 
            (*head_ref2) = (*head_ref2)->next; 
        } 
        else if ((*head_ref1)->value > (*head_ref2)->value) {
            return 1;
        } else {
            return -1;
        }
    }

    printf("Two list are the same\n");
    return 0;
}

int main() {

    //Need assume fisrt alphabet is the same?? Because how to compare c->a->d->e->NULL and a->b->c->NULL ???

    //First list
    struct node *head_1 = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)
    append(&head_1, 'a');
    append(&head_1, 'p');
    append(&head_1, 'p');
    append(&head_1, 'd');
    printf("First list:");
    print_list(head_1);

    //Second list
    struct node *head_2 = NULL;
    append(&head_2, 'a');
    append(&head_2, 'p');
    append(&head_2, 'p');

    printf("Second list:");
    print_list(head_2);

    //Compare two lists
    int ret = compare(&head_1, &head_2);
    printf("return value is %d\n", ret);

    return 0;
}
