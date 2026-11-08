#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "linked_list.h"

list_node* create_node(void *value) {
    list_node *new_node = malloc(sizeof(list_node));
    assert(new_node != NULL && "ERROR: Failed to alllocate memory.");    

    new_node->data = value;
    new_node->next = NULL;
    return new_node;
}

list_node* add_to_list(list_node *list, void *value) {
    list_node *head = list;

    if (head == NULL) {
        head = create_node(value);
        return head;
    }

    while (list->next != NULL) {
        list = list->next;
    }

    list->next = create_node(value);

    return head;
}

void free_list(list_node *head) {
    while (head != NULL) {
        list_node *free_node = head;
        head = head->next;
        free(free_node);
    }
}