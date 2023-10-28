#ifndef PRINTS_H
#define PRINTS_H

void print_block_sizes(const int* block_sizes, int size);

void print_block_start_index(const int* block_start_indexes, int size);

void print_process_received_data_length(const char* received_data, int rank);

void print_found_pattern_indexes(int* found_indexes, int size, int rank);

void print_gathered_final_result(const int* found_pattern_indexes, int size, int root_rank);

#endif
