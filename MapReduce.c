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
#define REDUCE_DEST "reduce_output.txt"

const int KEY_VAL = 1;

FILE *fp;
char *content;
size_t cont_size;
int num_reducers = 3;

typedef struct {
    size_t key_size;
    size_t value_size;
    void *key;
    void *value;
} key_value;

int find_num_digits(int num) {
    int num_digits = 0;
    if (num == 0) {
        num_digits = 1;
    }
    else {
        while (num != 0) {
            num /= 10;
            num_digits++;
        }
    }

    return num_digits;
}

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

        // Build term from content.
        while (!isspace(content[cont_pos]) && !iscntrl(content[cont_pos])) {
            term[term_pos] = content[cont_pos]; // Copy characters to build terms. 

            term_pos++;
            cont_pos++;

            assert(term_pos <= MAX_TERM_SIZE && "ERROR: Content is too large.");
        }

        // Establish term, create key-value pair, and add it to buffer.
        if (term_pos != 0) {
            term[term_pos] = '\0';

            key_value *pair = malloc(sizeof(key_value));
            assert(pair != NULL && "ERROR: Failed to allocate memory");
            pair->value = malloc(sizeof(int));
            assert(pair->value != NULL && "ERROR: Failed to allocate memory.");

            pair->key = term; // Term is the key.
            pair->key_size = term_pos;
            memcpy((int *)pair->value, &KEY_VAL, sizeof(int)); // Value of each key.
            pair->value_size = find_num_digits(KEY_VAL);


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
        char *term = (char *)((key_value *)curr_pair->data)->key;
        size_t term_size = ((key_value *)curr_pair->data)->key_size;
        int value = *(int *)((key_value *)curr_pair->data)->value;
        size_t value_size = ((key_value *)curr_pair->data)->value_size;

        int reducer_id = partition(term, term_size);

        // Structure of intermediate-values file is "{reducer ID} {term} 
        // {character length of term} {value} {character length of value}".
        fprintf(fp, "%d ", reducer_id);
        fprintf(fp, "%s ", term);
        fprintf(fp, "%llu ", term_size);
        fprintf(fp, "%d ", value);
        fprintf(fp, "%llu\n", value_size);

        curr_pair = curr_pair->next;
    }

    fclose(fp);

    // Free buffer memory.
    list_node *del_pair = loc_buff;
    while (del_pair != NULL) {
        free(((key_value *)del_pair->data)->key);
        free(((key_value *)del_pair->data)->value);
        free((key_value *)del_pair->data);
        del_pair = del_pair->next;
    }
    free_list(loc_buff);
}

void shuffle_sort(int thread_id, list_node** redkv_list) {
    assert((fp = fopen(INTERM_VALS_DEST, "r")) != NULL && "ERROR: Failed to open file.");

    int tred_id;
    char term[MAX_TERM_SIZE];
    size_t term_size;
    int *val = malloc(sizeof(int));
    assert(val != NULL && "ERROR: Failed to allocate memory.");
    size_t val_size;

    // Store thread ID, term, term size, value and value size from
    // intermediate-values file to variables.
    while(fscanf(fp, "%d %s %llu %d %llu", &tred_id, &term, &term_size, val, &val_size) == 5) {
        // Identify if term belongs to the calling reducer.
        if (tred_id == thread_id) {
            // Add term and its value to the calling reducer's list.
            if (*redkv_list == NULL) {
                key_value *tvalues_list = malloc(sizeof(key_value));
                assert(tvalues_list != NULL && "ERROR: Failed to allocate memory.");
                tvalues_list->key = malloc(sizeof(char) * term_size);
                assert(tvalues_list != NULL && "ERROR: Failed to allocate memory.");
                tvalues_list->value = malloc(sizeof(list_node));
                assert(tvalues_list->value != NULL && "ERROR: Failed to allocate memory.");

                strcpy(tvalues_list->key, term);
                tvalues_list->key_size = term_size;
                tvalues_list->value = create_node(val);
                tvalues_list->value_size = 1;

                //printf("%s %llu %d %llu\n", tvalues_list->key, tvalues_list->key_size, *((int *)((list_node *)tvalues_list->value)->data), tvalues_list->value_size); 

                *redkv_list = create_node(tvalues_list);
            }
            else {
                // TODO: If the calling reducer's list is not empty, add the term and its value to the list.
                // TODO: OR if it is already in the list, only add the value to the list of the corresponding term.
            }
        }

    }

    fclose(fp);
}

void reduce_func() {

}

void reduce_task(int thread_id) {
    list_node *redkv_list = NULL;
    shuffle_sort(thread_id, &redkv_list);

    // TODO: Free memory.
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "en_US.UTF8");

    assert(argc == 2 && "ERROR: Must input file name as argument.");

    readf(argv[1], &content, &cont_size);
    map_task();
    reduce_task(1);

    free(content);

    return 0;
}