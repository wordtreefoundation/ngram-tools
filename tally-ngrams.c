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

#define TRANSACTION_SIZE 100 * 1024 * 1024

tkvdb* db = NULL;
tkvdb_params* params = NULL;
tkvdb_tr* transaction = NULL;
tkvdb_cursor* cursor = NULL;
const char* db_storage_path = NULL;

FILE* input_file;

int SHOW_OUTPUT = false;
int COUNT = false;
int VERBOSE = false;
int DEBUG = false;

int total_ngrams_emitted = 0;
int total_file_size = 0;

void begin_transaction();
void commit_transaction();

void open_db(const char *path)
{
    if (path != NULL)
    {
        db = tkvdb_open(path, NULL);
    }
    
  	params = tkvdb_params_create();
	  if (!params) {
        fprintf(stderr, "Can't create database parameters\n");
        exit(EXIT_FAILURE);
    }

    tkvdb_param_set(params, TKVDB_PARAM_TR_DYNALLOC, 0);
    tkvdb_param_set(params, TKVDB_PARAM_TR_LIMIT, TRANSACTION_SIZE);
    tkvdb_param_set(params, TKVDB_PARAM_ALIGNVAL, sizeof(uint64_t));

    transaction = tkvdb_tr_create(db, params);
}

void close_db()
{
    transaction->free(transaction);

    if (db != NULL)
    {
        tkvdb_close(db);
    }
}

void begin_transaction()
{
    if (DEBUG) fprintf(stderr, "begin_transaction()\n");
    transaction->begin(transaction);
}

void commit_transaction()
{
    if (DEBUG) fprintf(stderr, "commit_transaction()\n");
    TKVDB_RES rc = transaction->commit(transaction);
    if (rc != TKVDB_OK)
    {
        fprintf(stderr, "commit() failed with code %d\n", rc);
        close_db();
        exit(EXIT_FAILURE);
    }
}

void create_cursor()
{
    if (DEBUG) fprintf(stderr, "create_cursor()\n");
    cursor = tkvdb_cursor_create(transaction);
    if (!cursor)
    {
        fprintf(stderr, "can't create cursor\n");
        close_db();
        exit(EXIT_FAILURE);
    }
}

void free_cursor()
{
    if (DEBUG) fprintf(stderr, "free_cursor()\n");
    cursor->free(cursor);
}

void print_range(char *start_ptr, size_t len)
{
    assert(start_ptr != NULL);
    assert(len > 0);
    
    putchar(' ');
    putchar(' ');
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
        fprintf(stderr, "  key size: %zu\n", key.size);

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
        if (DEBUG)
            fprintf(stderr, "  new result: %" PRId64 "\n", *(uint64_t *)value.data);
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
    begin_transaction();
    create_cursor();

    char *key;
    size_t key_len;
    uint64_t val;

    TKVDB_RES rc = cursor->first(cursor);
    if (rc == TKVDB_EMPTY)
    {
        fprintf(stderr, "No results to show\n");
    }
    else if (rc != TKVDB_OK)
    {
        fprintf(stderr, "Can't show results: error %d\n", (int)rc);
    }
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

    free_cursor();
    // commit_transaction();
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

        if (DEBUG)
        {
            fprintf(stderr, "  %zu bytes: %s\n", read, line);
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

        whitespace_after = first_non_white_space;
        if (!COUNT)
        {
            // We're expecting an integer, find out where its digits end
            while (*whitespace_after != ' ' &&
                  *whitespace_after != '\t')
            {
                if (*whitespace_after == '\0')
                    break;
                whitespace_after++;
            }
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
                if (COUNT)
                {
                    tally = 1;
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
                }

                if (DEBUG)
                    fprintf(stderr, "  tally: %" PRId64 "\n", tally);

                if (!COUNT)
                {
                    // Skip whitespace between the number an the text (ngram)
                    whitespace_after++;
                    while (*whitespace_after == ' ' ||
                          *whitespace_after == '\t')
                    {
                        if (*whitespace_after == '\0')
                            break;
                        whitespace_after++;
                    }
                }

                if (*whitespace_after != '\0')
                {
                    size_t skipped = (size_t)(whitespace_after - line);
                    if (DEBUG)
                        fprintf(stderr, "  ngram: %s, skipped: %zu\n", whitespace_after, skipped);
                    emit_ngram(whitespace_after, read - skipped, tally);
                }
            }
        }
    }
    if (line)
        free(line);

}

static const char *const usage[] = {
    "tally-ngrams [options] [[--] files]",
    NULL,
    NULL,
};

int main(int argc, char const *argv[])
{
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic Options"),
        OPT_STRING('p', "persist", &db_storage_path, "on-disk key-value storage file (optional)"),
        OPT_BOOLEAN('c', "count", &COUNT, "count occurrences (no integer column expected)"),
        OPT_BOOLEAN('s', "show", &SHOW_OUTPUT, "show ngrams after processing"),
        OPT_BOOLEAN('v', "verbose", &VERBOSE, "print some detail as progress is made"),
        OPT_BOOLEAN('d', "debug", &DEBUG, "show debug info"),
        OPT_END(),
    };

    if (db_storage_path == NULL)
      SHOW_OUTPUT = true;

    struct argparse argparse;
    argparse_init(&argparse, options, usage, 0);
    argparse_describe(&argparse,
                      "\nA command-line utility to tally ngrams and send results to STDOUT or persist to disk.",
                      "\nExample: ./tally-ngrams book.ngrams");
    argc = argparse_parse(&argparse, argc, argv);

    open_db(db_storage_path);
    begin_transaction();

    if (argc == 0)
    {
        if (VERBOSE)
            fprintf(stderr, "Reading from STDIN\n");
        
        input_file = stdin;
        iterate_over_ngrams();
        fclose(input_file);
    }
    else
    {
        for (int i = 0; i < argc; i++)
        {
            const char *text_file_path = argv[i];
            if (VERBOSE)
                fprintf(stderr, "Reading file %s\n", text_file_path);
            
            input_file = open_file(text_file_path);
            iterate_over_ngrams();
            fclose(input_file);
        }
    }

    if (SHOW_OUTPUT)
    {
        if (VERBOSE) fprintf(stderr, "Showing results\n");
        dump_database();
    }

    commit_transaction();
    close_db();

    if (VERBOSE)
        fprintf(stderr, "Processed %d ngrams\n", total_ngrams_emitted);
    return 0;
}
