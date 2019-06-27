#!/bin/bash

cc -g -Wall -pedantic -Wextra -I./tkvdb \
    tkvdb/tkvdb.c \
    ngrams.c \
    readall.c \
    stream-increment.c \
    -o stream-increment