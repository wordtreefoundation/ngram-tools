#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "common.h"

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
