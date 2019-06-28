#!/bin/bash

echo "Building tally-ngrams..."
cc -g -Wall -pedantic -Wextra -I./tkvdb \
    tkvdb/tkvdb.c \
    ngrams.c \
    readall.c \
    tally-ngrams.c \
    -o tally-ngrams

echo "Building text-to-ngrams..."
cc -g -Wall -pedantic -Wextra \
    ngrams.c \
    readall.c \
    text-to-ngrams.c \
    -o text-to-ngrams

echo "Done."