#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 20
int size_used = 0;

struct node {
   int value;
   struct node *next;
};

/* For hash table implemet +++ */
struct hash_item {
   int value;
   int mark;
};

int hash_func(int key) {
    return (key%CAPACITY);
}

void init_table(struct hash_item **tb) {
    *tb = (struct hash_item *)malloc(sizeof(struct hash_item) * CAPACITY);
    for (int i=0; i<CAPACITY; i++) {
        (*tb)[i].value = 0;
        (*tb)[i].mark = 0;
        //printf("[%d] value=%d, mark=%d\n", i, (*tb)[i].value, (*tb)[i].mark);
    }
}

void display_table(struct hash_item **tb) {
    printf("\nCurrent hash table:\n");
    for (int i=0; i<CAPACITY; i++) {
        printf("[%d] value=%d, mark=%d\n", i, (*tb)[i].value, (*tb)[i].mark);
    }
}

void insert_item(struct hash_item **tb, int value) {
    int key = hash_func(value);

    if (size_used > CAPACITY) {
        printf("No space...\n");
        return;
    }

    if ((*tb)[key].mark == 0) { //No value stored at this key yet (tb[key] == *(tb+key))
        (*tb)[key].value = value;
        (*tb)[key].mark = 1;
        size_used++; 
        //printf("Success to insert item: value=%d at key=%d\n", value, key);
    } else if ((*tb)[key].value == value) { //Contains same value at this key already
        //printf("Already contains item: value=%d at key=%d\n", value, key);
    } else { //No space for this value
        //printf("Fail to insert item: value=%d at key=%d because it contains other value\n", value, key);
    }
}

void remove_item(struct hash_item **tb, int value) {
    int key = hash_func(value);

    if ((*tb)[key].mark == 0) { //this key dose not contain any value yet
        printf("Fail to remove item: value=%d does not exist\n", value);
    } else if ((*tb)[key].value == value) { //Contains same key already
        (*tb)[key].value = 0;
        (*tb)[key].mark = 0;
        printf("Success to remove item: value=%d at key:%d\n", value, key);
        size_used--;
    }
}

int find_item(struct hash_item **tb, int value) {
    int key = hash_func(value);

    if ((*tb)[key].mark == 0) {
        return 0;
    } else {
        return 1;
    }
}
/* For hash table implemet --- */

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

void delete_duplicate_using_hash(struct node **head_ref, struct hash_item **tb) {
    struct node *current = *head_ref;
    struct node *dup_node; //用來記重複的node

    if (*head_ref == NULL) return;

    //First node
    insert_item(tb, current->value);

    while(current->next != NULL) {
        if (find_item(tb, (current->next)->value)) { //Found duplicate
            //printf("Found duplicate: %d\n", (current->next)->value);
            dup_node = current->next;
            current->next = dup_node->next;
            free(dup_node);
        } else {
            insert_item(tb, current->next->value);
            current = current->next; //Move forward
        }
    }
}

int main() {
    struct node *head = NULL; //head存在的目的是為了從頭搜尋(因為每次都只能從頭開始找)

    struct hash_item *table = NULL;
    init_table(&table);
//    display_table(&table);

    //Make duplicate
    append(&head, 5);
    append(&head, 5);
    append(&head, 6);
    for (int i=5; i<10; i++)
        append(&head, i);

    //Make duplicate
    append(&head, 10);
    append(&head, 10);
    append(&head, 6);
    append(&head, 11);
    append(&head, 12);
    append(&head, 13);
    append(&head, 13);
    
    printf("\nCurrent unsorted list is :\n");
    print_list(head);

    //Do delete duplicate
    delete_duplicate_using_hash(&head, &table);
    display_table(&table);

    printf("\nAfter delete duplicate:\n");
    print_list(head);

    return 0;
}
