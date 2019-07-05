#!/bin/bash

# echo "Building tally-ngrams..."
# cc -Ofast -Wall -pedantic -Wextra -Wno-missing-field-initializers \
#     -I./argparse -F./argparse -I./tkvdb -F./tkvdb \
#     argparse/argparse.c \
#     tkvdb/tkvdb.c \
#     src/common.c \
#     src/ngrams.c \
#     src/readall.c \
#     src/tally-ngrams.c \
#     -o tally-ngrams

echo "Testing ngrams library..."
cc -Ofast -Wall -pedantic -Wextra -Wno-missing-field-initializers \
    src/common.c \
    src/ngrams.c \
    src/readall.c \
    test/test-ngrams.c \
    -o test/test-ngrams \
&& ./test/test-ngrams

echo "Done."