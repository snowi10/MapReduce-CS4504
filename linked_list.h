#ifndef LINKED_LIST_H
#define LINKED_LIST_H

typedef struct node {
    void *data;
    struct node *next;
} list_node;

list_node* create_node(void *value);

list_node* add_to_list(list_node *head, void *value); 

void free_list(list_node *head); 

#endif