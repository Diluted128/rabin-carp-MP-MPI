#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mpi.h>
#include <windows.h>
#include <omp.h>

#define ROOT_RANK 0
#define HASH_PRIME 101
#define ALPHABET_CHARACTERS_COUNT 256
#define PATTERN "ABC"

int* find_pattern_in_text(char pattern[], char txt[], int* found_indexes_count) {
    int h = 1;
    int i, j;
    int index = 0;
    int text_hash = 0;
    int pattern_hash = 0;
    int pattern_size = strlen(pattern);
    int text_size = strlen(txt);

    int* buffer = (int *)malloc((text_size / pattern_size) * sizeof(int));

    // The value of h would be "pow(d, M-1)%q"
    for (i = 0; i < pattern_size - 1; i++)
        h = (h * ALPHABET_CHARACTERS_COUNT) % HASH_PRIME;

    for (i = 0; i < pattern_size; i++) {
        pattern_hash = (ALPHABET_CHARACTERS_COUNT * pattern_hash + pattern[i]) % HASH_PRIME;
        text_hash = (ALPHABET_CHARACTERS_COUNT * text_hash + txt[i]) % HASH_PRIME;
    }

    #pragma omp parallel for shared(buffer, index, pattern_hash, text_hash, txt, pattern_size, text_size) private(i, j)  schedule(static) num_threads(8)
    for (i = 0; i <= text_size - pattern_size; i++) {
        int local_index = -1;

        if (pattern_hash == text_hash) {
            for (j = 0; j < pattern_size; j++) {
                if (txt[i + j] != pattern[j]) {
                    break;
                }
            }

            if (j == pattern_size) {
                local_index = i;
            }
        }

        if (local_index != -1) {
            #pragma omp critical
            {   
                buffer[index] = local_index;
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

    int* found_indexes = (int *)malloc((index + 1) * sizeof(int));
    *found_indexes_count = index;

    for (i = 0; i < index; i++) {
        found_indexes[i] = buffer[i];
    }

    free(buffer);

    return found_indexes;
}

int *get_block_sizes(int reminder_count, int number_of_blocks, int size_of_each_block)
{

    int *blocks_sizes = (int *)malloc(number_of_blocks * sizeof(int));

    for (int i = 0; i < number_of_blocks; i++)
    {
        blocks_sizes[i] = size_of_each_block;
    }

    blocks_sizes[number_of_blocks - 1] += reminder_count;

    return blocks_sizes;
}

int *get_block_start_indexes(int number_of_blocks, int size_of_each_block)
{
    int *blocks_sizes = (int *)malloc(number_of_blocks * sizeof(int));

    int start_index = 0;

    for (int i = 0; i < number_of_blocks; i++)
    {
        blocks_sizes[i] = start_index;
        start_index += size_of_each_block;
    }
    return blocks_sizes;
}

char *read_text_file()
{
    FILE *file = fopen("random_chars.txt", "rb");
    fseek(file, 0L, SEEK_END);
    long lSize = ftell(file);
    rewind(file);

    char *buffer = (char *)malloc(lSize + 1 * sizeof(char));

    fread(buffer, lSize, 1, file);
    fclose(file);
    return buffer;
}

void set_variables_if_pattern_was_not_found(int *found_indexes, int *found_indexes_count)
{
    free(found_indexes);
    found_indexes = (int *)malloc(sizeof(int));
    found_indexes[0] = -1;
    *found_indexes_count = 1;
}

int *calculate_receive_block_indexes(const int *counts, int num_procs)
{
    int *sw = (int *)malloc(num_procs * sizeof(int));

    int start_index_value = 0;
    for (int j = 0; j < num_procs; j++)
    {
        sw[j] = start_index_value;
        start_index_value += counts[j];
    }

    return sw;
}

int calculate_final_result_size(const int *counts, int size)
{
    int sum = 0;
    for (int i = 0; i < size; i++)
    {
        sum += counts[i];
    }
    return sum;
}

int main(int argc, char *argv[])
{
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    int i;
    int rank;
    int num_procs;

    // From this point runs in parallel
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char *pat;
    int reminder_count = 0;
    int found_indexes_count = 0;

    int pattern_size;
    int buff_size;
    int *buff;
    int max_size_of_each_block;
    int *sending_block_sizes;
    int *sending_block_indexes;
    int *received_block_indexes = (int *)malloc(num_procs * sizeof(int));
    int *received_block_sizes = (int *)malloc(num_procs * sizeof(int));
    int *found_indexes;
    char *received_block;
    char *txt;

    if (rank == ROOT_RANK)
    {
        // read the text file
        txt = read_text_file();
        pattern_size = strlen(PATTERN);
        pat = (char *)malloc(strlen(PATTERN));
        strcpy(pat, PATTERN);

        // Calculate size of each block
        int size_of_each_block = strlen(txt) / num_procs;
        max_size_of_each_block = size_of_each_block + num_procs + 1;

        // Calculate size of the division reminder
        reminder_count = strlen(txt) - (size_of_each_block * num_procs);

        sending_block_sizes = get_block_sizes(reminder_count, num_procs, size_of_each_block);
        sending_block_indexes = get_block_start_indexes(num_procs, size_of_each_block);

    }

    // Send expected size to each process
    MPI_Bcast(&max_size_of_each_block, 1, MPI_INT, ROOT_RANK, MPI_COMM_WORLD);

    // Send pattern size to each process
    MPI_Bcast(&pattern_size, 1, MPI_CHAR, ROOT_RANK, MPI_COMM_WORLD);

    if (rank != ROOT_RANK)
    {
        pat = (char *)malloc(pattern_size);
    }

    // Send pattern to each process
    MPI_Bcast(pat, pattern_size, MPI_CHAR, ROOT_RANK, MPI_COMM_WORLD);

    received_block = (char *)malloc(max_size_of_each_block * sizeof(char));

    // Send different sized txt blocks
    MPI_Scatterv(txt, sending_block_sizes, sending_block_indexes, MPI_CHAR, received_block, max_size_of_each_block, MPI_CHAR, ROOT_RANK, MPI_COMM_WORLD);

    found_indexes = find_pattern_in_text(pat, received_block, &found_indexes_count);

    if (found_indexes_count == 0)
    {
        set_variables_if_pattern_was_not_found(found_indexes, &found_indexes_count);
    }


    MPI_Gather(&found_indexes_count, 1, MPI_INT, received_block_sizes, 1, MPI_INT, ROOT_RANK, MPI_COMM_WORLD);

    if (rank == ROOT_RANK)
    {
        received_block_indexes = calculate_receive_block_indexes(received_block_sizes, num_procs);

        buff_size = calculate_final_result_size(received_block_sizes, num_procs);

        buff = (int *)malloc(buff_size * sizeof(int));
    }

    MPI_Gatherv(found_indexes,
                found_indexes_count,
                MPI_INT, buff, received_block_sizes, received_block_indexes, MPI_INT, ROOT_RANK, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == ROOT_RANK)
    {
        QueryPerformanceCounter(&end);
        double elapsed_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
        printf("Czas wykonania: %.9f sekund\n", elapsed_time);
    }
    MPI_Finalize();

    return 0;
}