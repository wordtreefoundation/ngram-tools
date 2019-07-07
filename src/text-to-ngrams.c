#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <assert.h>

#include "argparse.h"

#include "readall.h"
#include "ngrams.h"
#include "common.h"

// #define DEBUG 1

FILE *input_file;
int WORDS_PER_NGRAM = 1;

int total_ngrams_emitted = 0;
int total_file_size = 0;

void print_ngram(char *start_ptr, size_t len, size_t index)
{
#pragma unused(index)
    print_range(start_ptr, len);

#ifdef DEBUG
    fprintf(stderr, "%zu\n", index);
#endif

    // Count the ngram in memory
    total_ngrams_emitted++;
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
        OPT_INTEGER('n', "ngrams", &WORDS_PER_NGRAM, "(integer) ngrams to produce"),
        OPT_GROUP("Other Options"),
        OPT_BOOLEAN('v', "verbose", &VERBOSE, "print some detail as progress is made"),
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
        for_each_ngram_of_file(input_file, WORDS_PER_NGRAM, print_ngram);
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
            for_each_ngram_of_file(input_file, WORDS_PER_NGRAM, print_ngram);
            fclose(input_file);
        }
    }

    if (VERBOSE)
        fprintf(stderr, "  ngrams emitted: %d\n", total_ngrams_emitted);

    return 0;
}
