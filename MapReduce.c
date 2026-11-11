#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <locale.h>
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include "linked_list.h"

#define MAX_FILE_SIZE 8192
#define MAX_TERM_SIZE 1024

#define INTERM_VALS_DEST "interm_vals.txt"
#define REDUCE_DEST "reduce_output.txt"

#define DEF_NUM_MAP 10
#define DEF_NUM_RED 10

const int KEY_VAL = 1;

FILE *fp;
char *content;
size_t cont_size;
int num_mappers = 5;
int num_reducers;
int last_pos = 0;

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

key_value* create_term_kv (char *term, size_t term_size, int val) {
    key_value *term_values = malloc(sizeof(key_value));
    assert(term_values != NULL && "ERROR: Failed to allocate memory.");
    term_values->key = malloc(sizeof(char) * (term_size + 1));
    assert(term_values->key != NULL && "ERROR: Failed to allocate memory.");
    term_values->value = NULL;
    int *v = malloc(sizeof(int));
    assert(v != NULL && "ERROR: Failed to allocate memory.");

    memcpy(v, &val, (sizeof(int)));
    strcpy(term_values->key, term);
    term_values->key_size = term_size;
    term_values->value = create_node(v);
    term_values->value_size = find_num_digits(val);

    return term_values;
}

void shuffle_sort(int thread_id, list_node** redkv_list) {
    assert((fp = fopen(INTERM_VALS_DEST, "r")) != NULL && "ERROR: Failed to open file.");

    int tred_id;
    char term[MAX_TERM_SIZE];
    size_t term_size;
    int val; 
    size_t val_size;

    // Store thread ID, term, term size, value and value size from
    // intermediate-values file to variables for observation.
    while(fscanf(fp, "%d %s %lu %d %lu", &tred_id, term, &term_size, &val, &val_size) == 5) {
        // Identify if term belongs to the calling reducer.
        if (tred_id == thread_id) {
            // Add term and its value to the calling reducer's list if the list is empty.
            if (*redkv_list == NULL) {
                key_value *term_values = create_term_kv(term, term_size, val);

                *redkv_list = create_node(term_values);
            }
            else {
                list_node *curr = *redkv_list;
                bool is_in_list = false;

                // Identify if the term already exists in the calling reducer's list.
                while (curr != NULL) {
                    char *curr_term = (char *)((key_value *)curr->data)->key;
                    list_node *val_list = (list_node *)((key_value *)curr->data)->value;

                    // If the term already exists in the calling reducer's list,
                    // only add the value to the current term's list of values in the
                    // reducer's list.
                    if (strcmp(curr_term, term) == 0) {
                        is_in_list = true;
			int *v = malloc(sizeof(int));
			memcpy(v, &val, sizeof(int));
                        val_list = add_to_list(val_list, v);
                        ((key_value *)curr->data)->value_size += 1; 
                        break;
                    }
                    else {
                        curr = curr->next;
                    }
                }

                // Add term and its value to the end of the calling reducer's list
                // if it does not yet exist in the list.
                if (is_in_list == false) {
                    key_value *term_values = create_term_kv(term, term_size, val);

                    *redkv_list = add_to_list(*redkv_list, term_values);
                }
            }
        }

    }

    fclose(fp);
}

int reduce_func(char *term, list_node *values) {
    int val_sum = 0;

    // Sum up values of the term.
    list_node *curr_val = values;
    while (curr_val != NULL) {
        val_sum += *(int *)curr_val->data;
        curr_val = curr_val->next;
    }

    printf("%s %d\n", term, val_sum);
    
    return val_sum;
}

void reduce_task(int thread_id) {
    list_node *redkv_list = NULL;

    shuffle_sort(thread_id, &redkv_list);

    assert((fp = fopen(REDUCE_DEST, "a")) != NULL && "ERROR: Failed to open file.");

    // Sum up values of each term and output to file. 
    list_node *curr_node = redkv_list;
    while (curr_node != NULL) {
        key_value *curr_kv = ((key_value *)curr_node->data);
        char *curr_term = (char *)curr_kv->key;
        list_node *curr_val = curr_kv->value;

        int val_sum = reduce_func(curr_term, curr_val);

        fprintf(fp, "%s %d\n", curr_term, val_sum);

        curr_node = curr_node->next;
    }

    fclose(fp);

    // Free memory.
    list_node *del_node = redkv_list;
    while (del_node != NULL) {
	    list_node *temp_node = del_node;
	    del_node = del_node->next;

	    key_value *del_kv = ((key_value *)temp_node->data);
	    char *del_term = (char *)del_kv->key;
	    free(del_term);
		
	    list_node *vals_list = (list_node *)del_kv->value;
	    list_node *del_vals_list = vals_list;
	    while (del_vals_list != NULL) {
		    list_node *temp_vals = del_vals_list;
		    del_vals_list = del_vals_list->next;

		    free(temp_vals->data);
	    }

	    free_list(vals_list);
	    free(del_kv);
    }
    free_list(redkv_list);

}

int partition(char *term, size_t term_size) {
    int term_id = 0;

    // Get positive integer value of term.
    for (int i = 0; i < term_size; i++) {
        term_id += (unsigned int)term[i];
    }

    // Assign term to reducer using modulo method.
    int reducer_id = term_id % num_reducers;

    return reducer_id;
}  

void map_func(list_node **buff, int start, int end) {
    int cont_pos = start;
    int end_pos = end;

    // Iterate through characters of content.
    while (cont_pos < end_pos) {
        int term_pos = 0;
        char term[MAX_TERM_SIZE];
        assert(term != NULL && "ERROR: Failed to allocate memory.");

        // Build term from content.
        while (isprint(content[cont_pos]) && !isspace(content[cont_pos]) && cont_pos < cont_size - 1) {
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
	    pair->key = malloc(sizeof(char) * (term_pos + 1));

            strcpy((char *)pair->key, term); // Term is the key.
            pair->key_size = term_pos;
            memcpy((int *)pair->value, &KEY_VAL, sizeof(int)); // Value of each key.
            pair->value_size = find_num_digits(KEY_VAL);


            *buff = add_to_list(*buff, pair); // Add pair to buffer.
        }

        cont_pos++; // Move to next character.
    }
}

void map_task(int thread_id) {
    list_node *loc_buff = NULL; // Local buffer. 
    double end_pos;
    double start_pos;

    // Last thread must read to the end of content.
    if (thread_id == num_mappers - 1) {
        end_pos = cont_size - 1;  
    }
    else {
        end_pos = (double)(cont_size - 1)/num_mappers * (thread_id + 1);
    }

    // Determine start position and end position for each thread.
    start_pos = end_pos - (double)(cont_size - 1)/num_mappers;
    end_pos = floor(end_pos);
    start_pos = floor(start_pos);

    // Move starting position if it is in the middle of a word.
    while (start_pos != 0 && isprint(content[(int)start_pos - 1]) && !isspace(content[(int)start_pos - 1])) {
        start_pos++;
    }

    map_func(&loc_buff, start_pos, end_pos);

    assert((fp = fopen(INTERM_VALS_DEST, "a")) != NULL && "ERROR: Failed to open file.");

    // Partition each term to reducer and write results to file.
    list_node *del_pair = loc_buff;
    while (del_pair != NULL) {
        char *term = (char *)((key_value *)del_pair->data)->key;
        size_t term_size = ((key_value *)del_pair->data)->key_size;
        int value = *(int *)((key_value *)del_pair->data)->value;
        size_t value_size = ((key_value *)del_pair->data)->value_size;

        int reducer_id = partition(term, term_size);
        
        // Structure of intermediate-values file is "{reducer ID} {term} 
        // {character length of term} {value} {character length of value}".
        fprintf(fp, "%d ", reducer_id);
        fprintf(fp, "%s ", term);
        fprintf(fp, "%lu ", term_size);
        fprintf(fp, "%d ", value);
        fprintf(fp, "%lu\n", value_size);

        del_pair = del_pair->next;
    } 

    fclose(fp);

    // Free memory.
    list_node *curr_node = loc_buff;
    while (curr_node != NULL) {
        list_node *temp = curr_node;
        curr_node = curr_node->next;

        free(((key_value *)temp->data)->key);
        free(((key_value *)temp->data)->value);
        free((key_value *)temp->data);
    }
    free_list(loc_buff);
}

void readf(char *file_name, char **dest, size_t *dest_size) {
    assert((fp = fopen(file_name, "r")) != NULL && "ERROR: Failed to open file.");

    // Read file and store in buffer.
    char buff[MAX_FILE_SIZE];
    int char_num = fread(buff, sizeof(char), MAX_FILE_SIZE, fp);
    char_num += 1; // To include null terminator.
    assert(char_num <= MAX_FILE_SIZE && "ERROR: File size is too large.");

    buff[char_num] = '\0';

    // Copy content of buffer to destination variable.
    *dest = malloc(sizeof(char) * char_num);
    assert(*dest != NULL && "ERROR: Failed to allocate memory.");
    strcpy(*dest, buff);
    *dest_size = char_num;
    
    fclose(fp);
}

int input_split(char *file_name, int init_num_mappers) {
    readf(file_name, &content, &cont_size);

    // Establish number of mappers to use (must not be greater than content size).
    while (init_num_mappers > cont_size) {
        init_num_mappers--;
    }

    return init_num_mappers;
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");

    assert(argc == 4 && "ERROR: Must input file name, number of mappers, and number of reducers (3 arguments).");

    char *file_name = argv[1];
    num_mappers = atoi(argv[2]);
    num_reducers = atoi(argv[3]);

    // Use default numbers if input number of mappers and number of reducers is zero.
    if (num_mappers == 0) {
        num_mappers = DEF_NUM_MAP;
    }
    if (num_reducers == 0) {
        num_reducers = DEF_NUM_RED;
    }

    // Create new, empty intermediate-values file.
    assert((fp = fopen(INTERM_VALS_DEST, "w")) != NULL && "ERROR: Failed to open file.");
    fclose(fp);

    // Determine number of mappers to use based on size of file.
    input_split(file_name, num_mappers);

    // Map.
    for (int i = 0; i < num_mappers; i++) {
        map_task(i);
    }

    // Create new, empty output file.
    assert((fp = fopen(REDUCE_DEST, "w")) != NULL && "ERROR: Failed to open file.");
    fclose(fp);

    // Reduce.
    for (int i = 0; i < num_reducers; i++) {
        reduce_task(i);
    }

    free(content);

    return 0;
}
