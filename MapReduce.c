#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>
#include "linked_list.h"

#define MAX_FILE_SIZE 2048
#define MAX_TERM_SIZE 256

#define VALUE 1

#define INTERM_VALS_DEST "interm_vals.txt"

FILE *fp;
char *content;
size_t cont_size;

typedef struct {
    void *key;
    void *value;
} key_value;

void readf(char *file_name, char **dest, size_t *dest_size) {
    assert((fp = fopen(file_name, "r")) != NULL && "ERROR: Failed to open file.");

    // Read file and store in buffer.
    char buff[MAX_FILE_SIZE];
    int char_num = fread(buff, sizeof(char), MAX_FILE_SIZE, fp);
    assert(char_num + 1 <= MAX_FILE_SIZE && "ERROR: File size is too large.");

    // Send content of buffer to destination variable.
    *dest = malloc(sizeof(char) * char_num);
    assert(*dest != NULL && "ERROR: Failed to allocate memory.");
    strcpy(*dest, buff);
    *dest_size = char_num;
}

void partition(void *term, size_t term_size) {
    
}  

void map_func(list_node **buff) {
    int cont_pos = 0;

    // Iterate through characters of content.
    while (cont_pos < cont_size) {
        int term_pos = 0;
        char *term = malloc(sizeof(char) * MAX_TERM_SIZE);
        assert(term != NULL && "ERROR: Failed to allocate memory.");

        // Build terms from content.
        while (!isspace(content[cont_pos]) && !iscntrl(content[cont_pos])) {
            term[term_pos] = content[cont_pos]; // Copy characters to build terms. 

            term_pos++;
            cont_pos++;

            assert(term_pos <= MAX_TERM_SIZE && "ERROR: Content is too large.");
        }

        // Establish terms and add key-value pairs to buffer.
        if (term_pos != 0) {
            term[term_pos] = '\0';

            key_value *pair = malloc(sizeof(key_value));
            assert(pair != NULL && "ERROR: Failed to allocate memory");
            pair->key = term; // Term is the key.
            pair->value = (int *)VALUE; // Value of each key.
            *buff = add_to_list(*buff, pair); // Add pair to buffer.
        }

        cont_pos++; // Move to next character.
    }
}

void map_task() {
    list_node *loc_buff = NULL; 
    map_func(&loc_buff);

    list_node *curr_pair = loc_buff;
    while (curr_pair != NULL) {
        // TODO: partition.
        printf("%s ", (char *)((key_value *)curr_pair->data)->key);
        printf("%d\n", (int *)((key_value *)curr_pair->data)->value);
        curr_pair = curr_pair->next;
    }


    // Free memory.
    list_node *del_pair = loc_buff;
    while (del_pair != NULL) {
        free(((key_value *)del_pair->data)->key);
        free((key_value *)del_pair->data);
        del_pair = del_pair->next;
    }
    free_list(loc_buff);
}

void reduce_func() {

}

void reduce_task() {

}

int main(int argc, char *argv[]) {
    assert(argc == 2 && "ERROR: Must input file name as argument.");

    readf(argv[1], &content, &cont_size);
    map_task();

    free(content);

    return 0;
}