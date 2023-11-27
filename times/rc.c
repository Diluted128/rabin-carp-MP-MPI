#include <stdio.h> 
#include <string.h> 
#include <windows.h>

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

    for (i = 0; i < pattern_size; i++) { 
        pattern_hash = (ALPHABET_CHARACTERS_COUNT * pattern_hash + pattern[i]) % prime; 
        text_hash = (ALPHABET_CHARACTERS_COUNT * text_hash + txt[i]) % prime; 
    } 
  
    for (i = 0; i <= text_size - pattern_size; i++) { 
  
        if (pattern_hash == text_hash) { 
            for (j = 0; j < pattern_size; j++) { 
                if (txt[i + j] != pattern[j]) 
                    break; 
            } 

            if (j == pattern_size) {
                // printf("Pattern found at index %d \n", i); 
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


int main() 
{ 
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);

    char *txt;
    txt = read_text_file();
    char pat[] = "ABC"; 
    int q = 101; // A prime number 
    search(pat, txt, q); 

    QueryPerformanceCounter(&end);
    double elapsed_time = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
    printf("Czas wykonania: %.9f sekund\n", elapsed_time);

    return 0; 
} 