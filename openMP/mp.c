#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>
#include <windows.h>

#define HASH_PRIME 101
#define ALPHABET_CHARACTERS_COUNT 256

void search(char pattern[], char txt[], int prime) 
{ 
    int pattern_size = strlen(pattern); 
    int text_size = strlen(txt); 
    int i, j; 
    int pattern_hash = 0;
    int text_hash = 0; 
    int h = 1; 
  
    // The value of h would be "pow(d, M-1)%q" 
    for (i = 0; i < pattern_size - 1; i++) 
        h = (h * ALPHABET_CHARACTERS_COUNT) % prime; 

   #pragma omp parallel for private(i, j, pattern_hash, text_hash) shared(txt, pattern) schedule(static) num_threads(8)
    for (i = 0; i <= text_size - pattern_size; i++) { 
  
        for (j = 0; j < pattern_size; j++) { 
            pattern_hash = (ALPHABET_CHARACTERS_COUNT * pattern_hash + pattern[j]) % prime; 
            text_hash = (ALPHABET_CHARACTERS_COUNT * text_hash + txt[i + j]) % prime; 
        }

        // Compare the hash values and check for a pattern match
        if (pattern_hash == text_hash) { 
            int match = 1;
            // Check character by character for a match
            for (j = 0; j < pattern_size; j++) { 
                if (txt[i + j] != pattern[j]) {
                    match = 0;
                    break;
                }
            } 

            if (match) {
                #pragma omp critical
                {
                    // printf("Pattern found at index %d \n", i);
                }
            }
        } 
  
        // Calculate hash value for next window of text: Remove 
        // leading digit, add trailing digit 
        if (i < text_size - pattern_size) { 
            text_hash = (ALPHABET_CHARACTERS_COUNT * (text_hash - txt[i] * h) + txt[i + pattern_size]) % prime; 

            if (text_hash < 0) 
                text_hash = (text_hash + prime); 
        } 
    } 
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

int main() {
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    char *txt;
    txt = read_text_file();

    char pat[] = "ABC";
    int* found_indexes;

    // Wywołujemy funkcję do wyszukiwania wzorca w tekście
    search(pat, txt, 101);

    QueryPerformanceCounter(&end);
    double elapsed_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    printf("Czas wykonania: %.9f sekund\n", elapsed_time);

    // Zwalniamy zaalokowaną pamięć
    free(found_indexes);

    return 0;
}
