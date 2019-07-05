#ifndef __NGRAMS_H__
#define __NGRAMS_H__

#include <stdio.h>
#include <stdint.h>

size_t text_clean_cstr(char *text);
void for_each_ngram_of_file(FILE*, int, void (*on_ngram)(char*, size_t, size_t));

#endif
