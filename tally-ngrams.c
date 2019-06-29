#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <assert.h>

#include "tkvdb/tkvdb.h"
#include "argparse/argparse.h"
#include "common.h"

FILE *input_file;
tkvdb_tr *transaction;
const char *db_storage_path = NULL;
int VERBOSE = false;
int SHOW_OUTPUT = false;
int DEBUG = false;

int total_ngrams_emitted = 0;
int total_file_size = 0;

void with_db(const char *path, void (*action)(void))
{
    tkvdb *db = NULL;

    if (path != NULL)
    {
        db = tkvdb_open(path, NULL);
    }
    transaction = tkvdb_tr_create(db, NULL);

    transaction->begin(transaction);
    (*action)();
    transaction->commit(transaction);

    transaction->free(transaction);

    if (path != NULL)
    {
        tkvdb_close(db);
    }
}

FILE *open_file(const char *path)
{
    FILE *file = fopen(path, "r");
    if (file == NULL)
    {
        fprintf(stderr, "  failed to open file: %s\n", path);
        exit(EXIT_FAILURE);
    }

    return file;
}

void print_range(char *start_ptr, size_t len)
{
    assert(start_ptr != NULL);
    assert(len > 0);

    char *end_ptr = start_ptr + len;
    while (start_ptr <= end_ptr)
    {
        putchar(*start_ptr++);
    }
    putchar('\n');
}

void emit_ngram(char *start_ptr, size_t len, uint64_t count)
{
    assert(start_ptr != NULL);
    if (len == 0 || count == 0)
        return;

    if (DEBUG)
        print_range(start_ptr, len);

    tkvdb_datum key, value;
    TKVDB_RES result;

    key.data = start_ptr;
    key.size = len;

    if (DEBUG)
        fprintf(stderr, "key.size: %zu\n", key.size);

    // All values will be 64-bit unsigned integers
    value.size = sizeof(uint64_t);

    if (transaction == NULL)
    {
        fprintf(stderr, "  null transaction\n");
        exit(EXIT_FAILURE);
    }

    // Retrieve current value of the key, if any
    result = transaction->get(transaction, &key, &value);
    if (result == TKVDB_OK)
    {
        // We've seen this key before, increment its value
        if (DEBUG)
            fprintf(stderr, "  result retrieved: %" PRId64 "\n", *(uint64_t *)value.data);
        // Edit value in-place
        (*(uint64_t *)value.data) += count;
    }
    else
    {
        // This is the first-ever value for this key
        value.data = (uint64_t *)&count;
        if (DEBUG)
            fprintf(stderr, "  result initialized: %" PRId64 "\n", *(uint64_t *)value.data);
        // Add new key-value pair
        transaction->put(transaction, &key, &value);
    }

    total_ngrams_emitted++;
}

void dump_database(void)
{
    TKVDB_RES rc;

    tkvdb_cursor *cursor = tkvdb_cursor_create(transaction);

    char *key;
    size_t key_len;
    uint64_t val;

    rc = cursor->first(cursor);
    while (rc == TKVDB_OK)
    {
        key = cursor->key(cursor);
        key_len = cursor->keysize(cursor);
        val = *(uint64_t *)cursor->val(cursor);

        printf("%7" PRId64 " ", val);
        fwrite(key, sizeof(char), key_len, stdout);
        putc('\n', stdout);

        rc = cursor->next(cursor);
    }

    cursor->free(cursor);
}

void iterate_over_ngrams(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    char *first_non_white_space;
    char *whitespace_after;
    uint64_t tally;
    while ((read = getline(&line, &len, input_file)) != -1)
    {
        // Remove newline at end of line, if present
        if (read > 0 && line[read - 1] == '\n')
            line[--read] = '\0';

        if (VERBOSE)
        {
            fprintf(stderr, "%zu bytes: %s\n", read, line);
        }

        // Skip any whitespace at beginning of line
        first_non_white_space = line;
        while (*first_non_white_space == ' ' ||
               *first_non_white_space == '\t')
        {
            if (*first_non_white_space == '\0')
                break;
            first_non_white_space++;
        }

        // We're expecting an integer, find out where its digits end
        whitespace_after = first_non_white_space;
        while (*whitespace_after != ' ' &&
               *whitespace_after != '\t')
        {
            if (*whitespace_after == '\0')
                break;
            whitespace_after++;
        }

        if (*first_non_white_space == '\0' || *whitespace_after == '\0')
        {
            if (DEBUG)
                fprintf(stderr, "  skipping line: %s\n", line);
        }
        else
        {
            if (*whitespace_after == '\0')
            {
                if (DEBUG)
                    fprintf(stderr, "  skipping line without ngram: %s\n", line);
            }
            else
            {
                // Put an EOL where we found whitespace after the digits, so we can conver to num
                *whitespace_after = '\0';
                uintmax_t num = strtoumax(first_non_white_space, NULL, 10);
                if (num == UINTMAX_MAX && errno == ERANGE)
                {
                    fprintf(stderr, "  could not convert to integer: %s\n", first_non_white_space);
                    exit(EXIT_FAILURE);
                }

                // We have the count!
                tally = (uint64_t)num;

                if (VERBOSE)
                    fprintf(stderr, "tally: %" PRId64 "\n", tally);

                // Skip whitespace between the number an the text (ngram)
                whitespace_after++;
                while (*whitespace_after == ' ' ||
                       *whitespace_after == '\t')
                {
                    if (*whitespace_after == '\0')
                        break;
                    whitespace_after++;
                }

                if (*whitespace_after != '\0')
                {
                    size_t skipped = (size_t)(whitespace_after - line);
                    if (VERBOSE)
                        fprintf(stderr, "ngram: %s\n", whitespace_after);
                    if (VERBOSE)
                        fprintf(stderr, "skipped: %zu\n", skipped);
                    emit_ngram(whitespace_after, read - skipped, tally);
                }
            }
        }
    }
    if (line)
        free(line);

    if (db_storage_path == NULL || SHOW_OUTPUT)
    {
        dump_database();
    }
}

static const char *const usage[] = {
    "tally-ngrams [options] [[--] files]",
    NULL,
    NULL,
};
// printf("USAGE: tally-ngrams <db-file> [text-file]\n\n");
// printf("  e.g. $ ./tally-ngrams baseline.tkvdb book.txt\n");
// printf("    or $ cat book.txt | ./tally-ngrams baseline.tkvdb\n");

int main(int argc, char const *argv[])
{
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic Options"),
        OPT_STRING('p', "persist", &db_storage_path, "on-disk key-value storage file (optional)"),
        OPT_BOOLEAN('s', "show", &SHOW_OUTPUT, "show ngram stored in database"),
        OPT_BOOLEAN('v', "verbose", &VERBOSE, "print some detail as progress is made"),
        OPT_BOOLEAN('d', "debug", &DEBUG, "show debug info"),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usage, 0);
    argparse_describe(&argparse,
                      "\nA command-line utility to tally ngrams and send results to STDOUT or persist to disk.",
                      "\nExample: ./tally-ngrams book.ngrams");
    argc = argparse_parse(&argparse, argc, argv);

    if (argc == 0)
    {
        input_file = stdin;
        if (VERBOSE)
            fprintf(stderr, "Reading from STDIN\n");
        with_db(db_storage_path, iterate_over_ngrams);
        fclose(input_file);
    }
    else
    {
        for (int i = 0; i < argc; i++)
        {
            const char *text_file_path = argv[i];
            input_file = open_file(text_file_path);
            if (VERBOSE)
                fprintf(stderr, "Reading file %s\n", text_file_path);
            with_db(db_storage_path, iterate_over_ngrams);
            fclose(input_file);
        }
    }

    if (VERBOSE)
        fprintf(stderr, "ngrams emitted: %d\n", total_ngrams_emitted);
    return 0;
}
