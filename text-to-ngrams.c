#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <assert.h>

#include "argparse/argparse.h"
#include "readall.h"
#include "ngrams.h"
#include "common.h"

FILE *input_file;
int NGRAMS = 1;
int VERBOSE = false;
int DEBUG = false;

int total_ngrams_emitted = 0;
int total_file_size = 0;

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
    if (VERBOSE)
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
                    if (read - word_boundary[0] > 0)
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

static const char *const usage[] = {
    "text-to-ngrams [options] [[--] files]",
    NULL,
    NULL,
};

int main(int argc, char const *argv[])
{
    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_GROUP("Basic Options"),
        OPT_INTEGER('n', "ngrams", &NGRAMS, "(integer) ngrams to produce"),
        OPT_BOOLEAN('v', "verbose", &VERBOSE, "print some detail as progress is made"),
        OPT_BOOLEAN('d', "debug", &DEBUG, "show debug info"),
        OPT_END(),
    };

    struct argparse argparse;
    argparse_init(&argparse, options, usage, 0);
    argparse_describe(&argparse,
                      "\nA command-line utility to convert a text file into a list of ngrams.",
                      "\nExample: ./text-to-ngrams -n4 book.txt");
    argc = argparse_parse(&argparse, argc, argv);

    if (argc == 0)
    {
        if (VERBOSE)
            fprintf(stderr, "Reading from STDIN\n");

        input_file = stdin;
        for_each_ngram_of_file(NGRAMS);
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
            for_each_ngram_of_file(NGRAMS);
            fclose(input_file);
        }
    }

    if (VERBOSE)
        fprintf(stderr, "  ngrams emitted: %d\n", total_ngrams_emitted);
    
    return 0;
}
