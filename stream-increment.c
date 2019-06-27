#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tkvdb/tkvdb.h"

const char *dbPath;
tkvdb_tr *transaction;

int with_db(const char *path, void (*action)(tkvdb_tr *))
{
    tkvdb *db = tkvdb_open(path, NULL);
    transaction = tkvdb_tr_create(db, NULL);

    transaction->begin(transaction);
    (*action)(transaction);
    transaction->commit(transaction);

    transaction->free(transaction);
    tkvdb_close(db);
}

FILE *file_open(const char *path)
{
    FILE *fp;

    fp =
        path == NULL
            ? stdin
            : fopen(path, "r");

    if (fp == NULL)
    {
        printf("Failed to open file.\n");
        exit(EXIT_FAILURE);
    }

    return fp;
}

void process_lines(FILE *openFile)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    tkvdb_datum key, value;
    TKVDB_RES result;

    // All values will be 64-bit unsigned integers
    value.size = sizeof(uint16_t);

    if (transaction == NULL)
    {
        printf("null transaction\n");
        exit(EXIT_FAILURE);
    }

    while ((read = getline(&line, &len, openFile)) != -1)
    {
        key.data = (void *)line;
        key.size = strlen(line);

        printf("line: %s\n", line);

        // Retrieve current value of the key, if any
        result = transaction->get(transaction, &key, &value);
        if (result == TKVDB_OK)
        {
            printf("result retrieved: %zu\n", *(uint64_t *)value.data);
            uint64_t newValue = *(uint64_t *)value.data;
            newValue++;
            value.data = (void *)&newValue;
        }
        else
        {
            printf("result not retrieved\n");
            // In case this is the first-ever value for this key
            uint64_t zeroValue = 0;
            value.data = (uint64_t *)&zeroValue;
            printf("result initialized: %zu\n", *(uint64_t *)value.data);
        }
        transaction->put(transaction, &key, &value);
    }

    if (line)
    {
        free(line);
    }
}

void process_file(FILE *openFile)
{
    const char *path = dbPath;
    with_database(path, process_lines);
}

int main(int argc, char const *argv[])
{
    if (argc == 1)
    {
        printf("USAGE: cat textfile | stream-increment <DB FILE>\n");
    }
    else if (argc >= 2)
    {
        const char *text_file_path = argc == 3 ? argv[2] : NULL;
        file_open(text_file_path);

        const char *db_storage_path = argv[1];
        with_db(db_storage_path, process_file);
    }

    return 0;
}
