#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "common.h"
#include "readall.h"

/** Transforms text such as the following:
 *
 *   And behold, I said, "This is no good!"
 *   What shall ye say unto these people, there-
 *   fore?
 *
 * Into a cleaned up single line of text, like the following:
 *
 *   and behold i said this is no good.what shall ye say unto these people therefore.
 *
 * Spaces indicate word boundaries, while periods indicate sentence boundaries.
 */
size_t text_clean_cstr(char *text)
{
    if (*text == '\0')
        return 0;

    char *read;
    char *write = text;
    uint8_t join_lines = false,
            just_added_space = true, // prevent prefix spaces
        just_added_period = false;
    for (read = text; *read; read++)
    {
        char c = *read;
        if (c >= 'A' && c <= 'Z')
        {
            // Change upper case to lowercase
            c += 32;
        }
        else if (c == '\n' || c == '(' || c == ')')
        {
            // Change newlines & parens to spaces (i.e. they count as word boundaries)
            c = ' ';
        }
        else if (c == '?' || c == '!')
        {
            // Change exclamation, question marks to periods (i.e. sentence boundaries)
            c = '.';
        }

        if (c == '-')
        {
            join_lines = true;
        }
        else if (join_lines && c == ' ')
        {
            // ignore whitespace after a dash (i.e. including newlines, which is the
            // most common case because words that are broken by syllables are dashed)
        }
        else if (c == '.' && !just_added_period)
        {
            // erase space before period
            if (just_added_space)
                write--;
            *write++ = '.';
            just_added_period = true;
            just_added_space = false;
            join_lines = false;
        }
        else if (c == ' ' && !just_added_space && !just_added_period)
        {
            *write++ = ' ';
            just_added_space = true;
            just_added_period = false;
        }
        else if (c >= 'a' && c <= 'z')
        {
            *write++ = c;
            just_added_space = false;
            just_added_period = false;
            join_lines = false;
        }
    }
    // erase space at end of text
    if (just_added_space)
        write--;

    // Return the new length of the string
    return (size_t)(write - text);
}

/**
 * FILE* input_file: an open file ready to be read by `readall` for ascii data
 * int ngram_size: the number of words to collect and emit for `on_ngram` (e.g. 4)
 * function on_ngram: a callback function that will be called on each ngram
 *   callback arguments:
 *     char* ngram: a pointer to the ngram string (NOT null terminated)
 *     size_t len: the length of the ngram string
 *     size_t index: the iteration number of the ngram, relative to the whole loop
 */
void for_each_ngram_of_file(
    FILE* input_file,
    int ngram_size,
    void (*on_ngram)(char*, size_t, size_t))
{
    char *content = NULL;
    size_t len = 0;

    int result = readall(input_file, &content, &len);

    assert(ngram_size < 32);

    if (VERBOSE)
        fprintf(stderr, "size (bytes): %zu\n", len);

    if (result == READALL_OK)
    {
        #ifdef DEBUG
        printf("FILE READ SUCCESSFUL\n");
        #endif

        len = text_clean_cstr(content);
        content[len] = '\0';

        #ifdef DEBUG
        printf("%s\n", content);
        #endif

        int iter = 0;
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
                    {
                        const size_t ngram_length = read - word_boundary[0];
                        on_ngram(
                          word_boundary[0],
                          ngram_length - 1,
                          iter++
                        );
                    }

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
