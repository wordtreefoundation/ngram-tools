#!/bin/bash

cc -g -Wall -pedantic -Wextra -I./tkvdb \
    tkvdb/tkvdb.c \
    ngrams.c \
    readall.c \
    tally-ngrams.c \
    -o tally-ngrams