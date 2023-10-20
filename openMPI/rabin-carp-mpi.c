#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <mpi.h>

#define ROOT_RANK 0
#define HASH_PRIME 101
#define ALPHABET_CHARACTERS_COUNT 256

int* find_pattern_in_text(char pattern[], char txt[]){ 
    int pattern_size = strlen(pattern); 
    int text_size = strlen(txt); 
    int index = 0;
    int i, j; 
    int pattern_hash = 0;
    int text_hash = 0; 
    int h = 1; 
    int* found_indexes = (int *)malloc(10 * sizeof(int));
 
    // The value of h would be "pow(d, M-1)%q" 
    for (i = 0; i < pattern_size - 1; i++) 
        h = (h * ALPHABET_CHARACTERS_COUNT) % HASH_PRIME; 

    for (i = 0; i < pattern_size; i++) { 
        pattern_hash = (ALPHABET_CHARACTERS_COUNT * pattern_hash + pattern[i]) % HASH_PRIME; 
        text_hash = (ALPHABET_CHARACTERS_COUNT * text_hash + txt[i]) % HASH_PRIME; 
    } 
  
    for (i = 0; i <= text_size - pattern_size; i++) { 
  
        if (pattern_hash == text_hash) { 
            for (j = 0; j < pattern_size; j++) { 
                if (txt[i + j] != pattern[j]) 
                    break; 
            } 

            if (j == pattern_size){
                found_indexes[index] = i;
                index++;
            }
        } 
  
        // Calculate hash value for next window of text: Remove 
        // leading digit, add trailing digit 
        if (i < text_size - pattern_size) { 
            text_hash = (ALPHABET_CHARACTERS_COUNT * (text_hash - txt[i] * h) + txt[i + pattern_size]) % HASH_PRIME; 

            if (text_hash < 0) 
                text_hash = (text_hash + HASH_PRIME); 
        } 
    } 

    return found_indexes;
} 

int* get_block_sizes(int reminder_count, int number_of_blocks, int size_of_each_block){

    int i;
    int* blocks_sizes;

    blocks_sizes = (int *)malloc(number_of_blocks * sizeof(int));

    for(int i=0; i<number_of_blocks; i++){
        blocks_sizes[i] = size_of_each_block;
    }

    blocks_sizes[number_of_blocks - 1] += reminder_count;

    return blocks_sizes;
}

int* get_block_start_indexes(int number_of_blocks, int size_of_each_block){
    int* blocks_sizes = (int *)malloc(number_of_blocks * sizeof(int));
    
    int start_index = 0;
    for(int i=0; i<number_of_blocks; i++){
        blocks_sizes[i] = start_index;
        start_index += size_of_each_block;
    }
    return blocks_sizes;
}

char* read_text_file(){
    FILE *file = fopen("text.txt", "rb");
    fseek( file , 0L , SEEK_END);
    long lSize = ftell( file );
    rewind( file );

    char* buffer = (char *)malloc(lSize+1 * sizeof(char));

    fread( buffer , lSize, 1 , file);
    fclose(file);
    return buffer;
}

void print_block_sizes(int* block_sizes, int size){
    printf("Block sizes: ");
    for(int i=0; i<size; i++){
        printf("%d ", block_sizes[i]);
    }
    printf("\n");
}

void print_block_start_index(int* block_start_indexes, int size){
    printf("Block start indexes: ");
    for(int i=0; i<size; i++){
        printf("%d ", block_start_indexes[i]);
    }
    printf("\n");
}

void print_process_received_data(char* received_data, int rank){
    printf("Procesor %d got %lu : ", rank, strlen(received_data));
    for(int i=0; i<strlen(received_data); i++){
        printf("%c", received_data[i]);
    }
    printf("\n");
}

void print_found_pattern_indexes(int* found_indexes, int size, int rank){
    printf("Procesor %d calculated: ", rank);
    for(int i=0; i<size; i++){
        printf("%d ", found_indexes[i]);
    }
    printf("\n");
}

// ToDo 
// - pass the pattern to specific process
// - MPI_Gather
int main(int argc, char* argv[]){    
    
    int rank;
    int num_procs;
    int reminder_count = 0;
    int found_indexes_count;
    int* block_sizes;
    int* block_start_indexes;
    int* displacments;
    int* found_indexes;
    char received_data[200];
    char pat[] = "Lorem";
    char* txt;

    // From this point runs in parallel
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    int size_of_each_block;

    if(rank == ROOT_RANK){
        txt = read_text_file();
  
        printf("Number of procs %d\n", num_procs);
        printf("Text size: %lu\n", strlen(txt));

        // Calculate size of each block
        size_of_each_block = strlen(txt) / (num_procs);

        // Calculate size of the division reminder count 
        reminder_count = strlen(txt) - (size_of_each_block * num_procs);

        // Array of the block sizes
        block_sizes = get_block_sizes(reminder_count, num_procs, size_of_each_block);
        
        // Array of the block start indexes
        block_start_indexes = get_block_start_indexes(num_procs, size_of_each_block);
        
        print_block_sizes(block_sizes, num_procs);

        print_block_start_index(block_start_indexes, num_procs);
    }
    
    // Send pattern to each process
    MPI_Bcast(&pat, 1, MPI_CHAR, ROOT_RANK, MPI_COMM_WORLD);

    // Send different sized txt blocks
    MPI_Scatterv(txt, block_sizes, block_start_indexes, MPI_CHAR, &received_data, 200, MPI_CHAR, ROOT_RANK, MPI_COMM_WORLD);

    print_process_received_data(received_data, rank);

    found_indexes = find_pattern_in_text(pat, received_data);

    print_found_pattern_indexes(found_indexes, 10, rank);
    
    MPI_Finalize();

    return 0;
}