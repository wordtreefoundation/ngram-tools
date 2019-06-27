#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tkvdb/tkvdb.h"
#include "readall.h"
#include "ngrams.h"
#include "common.h"

FILE *input_file;
tkvdb_tr *transaction;
int DEBUG = false;

int total_ngrams_emitted = 0;
int total_file_size = 0;

void with_db(const char *path, void (*action)(void))
{
    tkvdb *db = tkvdb_open(path, NULL);
    transaction = tkvdb_tr_create(db, NULL);

    transaction->begin(transaction);
    (*action)();
    transaction->commit(transaction);

    transaction->free(transaction);
    tkvdb_close(db);
}

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

    if (DEBUG)
        print_range(start_ptr, end_ptr);

    tkvdb_datum key, value;
    TKVDB_RES result;

    key.data = start_ptr;
    key.size = (size_t)(end_ptr - start_ptr);

    // All values will be 64-bit unsigned integers
    value.size = sizeof(uint16_t);

    if (transaction == NULL)
    {
        printf("null transaction\n");
        exit(EXIT_FAILURE);
    }

    // Retrieve current value of the key, if any
    result = transaction->get(transaction, &key, &value);
    if (result == TKVDB_OK)
    {
        // We've seen this key before, increment its value
        if (DEBUG)
            printf("result retrieved: %zu\n", *(uint64_t *)value.data);
        (*(uint64_t *)value.data)++;
    }
    else
    {
        // In case this is the first-ever value for this key
        uint64_t initialValue = 1;
        value.data = (uint64_t *)&initialValue;
        if (DEBUG)
            printf("result initialized: %zu\n", *(uint64_t *)value.data);
    }

    // Store off the result
    transaction->put(transaction, &key, &value);

    total_ngrams_emitted++;
}

void for_each_ngram_of_file(void)
{
    char *content = NULL;
    size_t len = 0;

    int result = readall(input_file, &content, &len);

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

        int ngram_size = 4;
        assert(ngram_size < 32);

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
        printf("USAGE: stream-increment <db-file> [text-file]\n\n");
        printf("  e.g. $ stream-increment baseline.tkvdb book.txt\n");
        printf("    or $ cat book.txt | stream-increment baseline.tkvdb\n");
    }
    else if (argc >= 2)
    {
        const char *text_file_path = NULL;
        if (argc == 3)
        {
            text_file_path = argv[2];
            fprintf(stderr, "stream-increment: %s\n", text_file_path);
        }
        else
        {
            fprintf(stderr, "stream-increment: STDIN\n");
        }
        file_open(text_file_path);

        const char *db_storage_path = argv[1];
        with_db(db_storage_path, for_each_ngram_of_file);
    }

    fprintf(stderr, "ngrams emitted: %d\n", total_ngrams_emitted);
    return 0;
}
