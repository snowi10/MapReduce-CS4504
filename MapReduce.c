#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <locale.h>
#include "linked_list.h"

#define MAX_FILE_SIZE 2048
#define MAX_TERM_SIZE 256

#define INTERM_VALS_DEST "interm_vals.txt"

FILE *fp;
char *content;
size_t cont_size;
int num_reducers = 3;
int KEY_VAL = 1;

typedef struct {
    size_t key_size;
    size_t value_size;
    void *key;
    void *value;
} __key_value;

void readf(char *file_name, char **dest, size_t *dest_size) {
    assert((fp = fopen(file_name, "r")) != NULL && "ERROR: Failed to open file.");

    // Read file and store in buffer.
    char buff[MAX_FILE_SIZE];
    int char_num = fread(buff, sizeof(char), MAX_FILE_SIZE, fp);
    assert(char_num <= MAX_FILE_SIZE && "ERROR: File size is too large.");

    // Copy content of buffer to destination variable.
    *dest = malloc(sizeof(char) * char_num);
    assert(*dest != NULL && "ERROR: Failed to allocate memory.");
    strcpy(*dest, buff);
    *dest_size = char_num;

    fclose(fp);
}

int partition(char *term, size_t term_size) {
    int term_id = 0;

    // Get positive integer value of term.
    for (int i = 0; i < term_size; i++) {
        term_id = (unsigned int)term[i];
    }

    // Assign term to reducer using modulo method.
    int reducer_id = term_id % num_reducers;

    return reducer_id;
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

            __key_value *pair = malloc(sizeof(__key_value));
            assert(pair != NULL && "ERROR: Failed to allocate memory");
            pair->key = term; // Term is the key.
            pair->key_size = term_pos;
            pair->value = &KEY_VAL; // Value of each key.
            pair->value_size = sizeof(int);

            *buff = add_to_list(*buff, pair); // Add pair to buffer.
        }

        cont_pos++; // Move to next character.
    }
}

void map_task() {
    list_node *loc_buff = NULL; // Local buffer. 
    map_func(&loc_buff);

    assert((fp = fopen(INTERM_VALS_DEST, "w")) != NULL && "ERROR: Failed to open file.");

    // Partition each term to reducer and write results to file.
    list_node *curr_pair = loc_buff;
    while (curr_pair != NULL) {
        char *term = (char *)((__key_value *)curr_pair->data)->key;
        size_t term_size = ((__key_value *)curr_pair->data)->key_size;
        int value = *(int *)((__key_value *)curr_pair->data)->value;

        int reducer_id = partition(term, term_size);

        fprintf(fp, "%d ", reducer_id);
        fprintf(fp, "%s ", term);
        fprintf(fp, "%d\n", value);

        curr_pair = curr_pair->next;
    }

    fclose(fp);

    // Free buffer memory.
    list_node *del_pair = loc_buff;
    while (del_pair != NULL) {
        free(((__key_value *)del_pair->data)->key);
        free((__key_value *)del_pair->data);
        del_pair = del_pair->next;
    }
    free_list(loc_buff);
}

void shuffle_sort(int thread_id) {
}

void reduce_func() {

}

void reduce_task(int thread_id) {

}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "en_US.UTF8");

    assert(argc == 2 && "ERROR: Must input file name as argument.");

    readf(argv[1], &content, &cont_size);
    map_task();

    free(content);

    return 0;
}