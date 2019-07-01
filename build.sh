#!/bin/bash

echo "Building tally-ngrams..."
cc -g -Wall -pedantic -Wextra -Wno-missing-field-initializers \
    -I./argparse -F./argparse -I./tkvdb -F./tkvdb \
    argparse/argparse.c \
    tkvdb/tkvdb.c \
    src/common.c \
    src/ngrams.c \
    src/readall.c \
    src/tally-ngrams.c \
    -o bin/tally-ngrams

echo "Building text-to-ngrams..."
cc -g -Wall -pedantic -Wextra -Wno-missing-field-initializers \
    -I./argparse -F./argparse \
    argparse/argparse.c \
    src/common.c \
    src/ngrams.c \
    src/readall.c \
    src/text-to-ngrams.c \
    -o bin/text-to-ngrams

echo "Done."