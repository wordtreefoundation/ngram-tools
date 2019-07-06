#!/bin/bash

{
  echo "Building tally-lines..."
  cc -Ofast -Wall -pedantic -Wextra -Wno-missing-field-initializers \
      -I./argparse -F./argparse -I./tkvdb -F./tkvdb \
      argparse/argparse.c \
      tkvdb/tkvdb.c \
      src/common.c \
      src/ngrams.c \
      src/readall.c \
      src/tally-lines.c \
      -o tally-lines
} && {
  echo "Building text-to-ngrams..."
  cc -Ofast -Wall -pedantic -Wextra -Wno-missing-field-initializers \
      -I./argparse -F./argparse \
      argparse/argparse.c \
      src/common.c \
      src/ngrams.c \
      src/readall.c \
      src/text-to-ngrams.c \
      -o text-to-ngrams
}
