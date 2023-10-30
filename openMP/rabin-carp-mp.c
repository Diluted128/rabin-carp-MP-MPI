#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>

#define HASH_PRIME 101
#define ALPHABET_CHARACTERS_COUNT 256

// Funkcja do wyszukiwania wzorca w tekście przy użyciu algorytmu Rabin-Karpa
int* find_pattern_in_text(char pattern[], char txt[]) {
    int pattern_size = strlen(pattern);    // Długość wzorca
    int text_size = strlen(txt);          // Długość tekstu
    int i, j;
    int pattern_hash = 0;                 // Hash wzorca
    int text_hash = 0;                   // Hash aktualnego okna tekstu
    int h = 1;                            // Wartość wykorzystywana do obliczenia hasza
    int* found_indexes = (int*)malloc(10 * sizeof(int)); // Tablica wyników
    int index = 0;                        // Indeks wyników

    // Obliczamy wartość h, która będzie wynosić "pow(ALPHABET_CHARACTERS_COUNT, pattern_size-1) % HASH_PRIME"
    for (i = 0; i < pattern_size - 1; i++) {
        h = (h * ALPHABET_CHARACTERS_COUNT) % HASH_PRIME;
    }

    // Rozpoczynamy równoległe przeszukiwanie tekstu przy użyciu OpenMP
    #pragma omp parallel for shared(found_indexes, index) private(i, j, pattern_hash, text_hash) schedule(static)
    for (i = 0; i <= text_size - pattern_size; i++) {
        pattern_hash = 0;
        text_hash = 0;

        // Obliczamy hasze wzorca i aktualnego okna tekstu
        for (j = 0; j < pattern_size; j++) {
            pattern_hash = (ALPHABET_CHARACTERS_COUNT * pattern_hash + pattern[j]) % HASH_PRIME;
            text_hash = (ALPHABET_CHARACTERS_COUNT * text_hash + txt[i + j]) % HASH_PRIME;
        }

        if (pattern_hash == text_hash) {
            for (j = 0; j < pattern_size; j++) {
                if (txt[i + j] != pattern[j])
                    break;
            }

            // Jeśli hasze są równe i wzorzec jest znaleziony, dodajemy indeks do wyników
            if (j == pattern_size) {
                #pragma omp critical
                {
                    found_indexes[index] = i;
                    index++;
                }
            }
        }
    }

    return found_indexes;
}

int main() {
    char txt[] = "GEEKS FOR GEEKS";
    char pat[] = "GEEK";
    int* found_indexes;

    // Wywołujemy funkcję do wyszukiwania wzorca w tekście
    found_indexes = find_pattern_in_text(pat, txt);

    // Wyświetlamy znalezione indeksy
    for (int i = 0; i < 10; i++) {
        if (found_indexes[i] >= 0 && found_indexes[i] <= 10) {
            printf("Pattern found at index %d\n", found_indexes[i]);
        }
    }

    // Zwalniamy zaalokowaną pamięć
    free(found_indexes);

    return 0;
}
