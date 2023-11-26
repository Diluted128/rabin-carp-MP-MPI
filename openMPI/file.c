#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Funkcja do generowania losowych znaków i zapisu do pliku
void generate_and_save_random_chars(const char *filename, long int count) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Nie można otworzyć pliku do zapisu.\n");
        return;
    }

    srand(time(NULL)); // Inicjalizacja generatora liczb pseudolosowych

    for (long int i = 0; i < count; ++i) {
        char random_char = 'A' + rand() % 26; // Losowy znak od 'A' do 'Z'
        fprintf(file, "%c", random_char); // Zapisuje każdy znak w osobnej linii
    }

    fclose(file);
}

int main() {
    const char *filename = "random_chars.txt"; // Nazwa pliku
    long int count = 1000000000; // Liczba losowych znaków do wygenerowania

    generate_and_save_random_chars(filename, count);

    return 0;
}
