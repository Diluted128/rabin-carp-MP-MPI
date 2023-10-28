#include "prints.h"
#include <stdio.h>
#include <string.h>

void print_block_sizes(const int* block_sizes, int size){
    printf("Characters blocks sizes: ");
    for(int i=0; i<size; i++){
        printf("%d ", block_sizes[i]);
    }
    printf("\n");
}

void print_block_start_index(const int* block_start_indexes, int size){
    printf("Blocks start indexes: ");
    for(int i=0; i<size; i++){
        printf("%d ", block_start_indexes[i]);
    }
    printf("\n");
}

void print_process_received_data_length(const char* received_data, int rank){
    printf("Processor %d got block with %lu characters..\n", rank, strlen(received_data));
}

void print_found_pattern_indexes(int* found_indexes, int size, int rank){
    if(size == 1 && found_indexes[0] == -1){
        printf("Processor %d has not found it... ", rank);
    }
    else{
        printf("Processor %d found pattern at index: ", rank);
        for(int i=0; i<size; i++){
            printf("%d, ", found_indexes[i]);
        }
    }
    printf("\n");
}

void print_gathered_final_result(const int* found_pattern_indexes, int size, int root_rank){
    printf("Master proc %d received final result: ", root_rank);
    for(int i=0; i<size; i++){
        printf("%d ", found_pattern_indexes[i]);
    }
    printf("\n");
}