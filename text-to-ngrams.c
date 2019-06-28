#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <assert.h>

#include "readall.h"
#include "ngrams.h"
#include "common.h"

FILE *input_file;
int DEBUG = false;

int total_ngrams_emitted = 0;
int total_file_size = 0;

FILE *file_open(const char *path)
{
    input_file =
        path == NULL
            ? stdin
            : fopen(path, "r");

    if (input_file == NULL)
    {
        printf("Failed to open file.\n");
        exit(EXIT_FAILURE);
    }

    return input_file;
}

void print_range(char *start_ptr, char *end_ptr)
{
    assert(start_ptr <= end_ptr);
    while (start_ptr <= end_ptr)
    {
        putchar(*start_ptr++);
    }
    putchar('\n');
}

void emit_ngram(char *start_ptr, char *end_ptr)
{
    assert(start_ptr <= end_ptr);

    print_range(start_ptr, end_ptr);

    // Count the ngram in memory
    total_ngrams_emitted++;
}

void for_each_ngram_of_file(int ngram_size)
{
    char *content = NULL;
    size_t len = 0;

    int result = readall(input_file, &content, &len);

    assert(ngram_size < 32);

    total_file_size = (int)len;
    fprintf(stderr, "size (bytes): %d\n", total_file_size);

    if (result == READALL_OK)
    {
        if (DEBUG)
            printf("FILE READ SUCCESSFUL\n");

        len = text_clean_cstr(content);
        content[len] = '\0';

        if (DEBUG)
            printf("%s\n", content);

        int word_count = 0;
        char *word_boundary[32];
        char *read;

        word_boundary[word_count++] = content;
        for (read = content; read <= content + len; read++)
        {
            if (*read == ' ' || *read == '.' || read == content + len)
            {
                word_boundary[word_count] = read + 1;
                if (word_count == ngram_size)
                {
                    // The important part!
                    emit_ngram(word_boundary[0], read - 1);

                    for (int i = 0; i < word_count; i++)
                    {
                        word_boundary[i] = word_boundary[i + 1];
                    }
                }
                else
                {
                    word_count++;
                }

                // Start new sentence
                if (*read == '.')
                {
                    word_count = 0;
                    word_boundary[word_count++] = read + 1;
                }
            }
        }
    }
    else
    {
        printf("Unable to read file into memory\n");
        exit(EXIT_FAILURE);
    }

    free(content);
}

int main(int argc, char const *argv[])
{
    if (argc == 1)
    {
        printf("USAGE: text-to-ngrams <ngrams> [text-file]\n\n");
        printf("  e.g. $ ./text-to-ngrams 4 book.txt\n");
        printf("    or $ cat book.txt | ./text-to-ngrams 4\n");
    }
    else if (argc >= 2)
    {
        const char *text_file_path = NULL;
        if (argc == 3)
        {
            text_file_path = argv[2];
            fprintf(stderr, "Input File: %s\n", text_file_path);
        }
        else
        {
            fprintf(stderr, "Input File: STDIN\n");
        }
        file_open(text_file_path);

        const char *ngrams = argv[1];
        uintmax_t num = strtoumax(ngrams, NULL, 10);
        if (num == UINTMAX_MAX && errno == ERANGE)
        {
            fprintf(stderr, "Could not convert to integer: %s\n", ngrams);
            exit(EXIT_FAILURE);
        }
        for_each_ngram_of_file(num);
    }

    fprintf(stderr, "  ngrams emitted: %d\n", total_ngrams_emitted);
    return 0;
}
