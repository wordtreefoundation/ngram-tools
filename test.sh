#!/bin/bash
RESTORE=$(echo -en '\001\033[0m\002')
RED=$(echo -en '\001\033[00;31m\002')
GREEN=$(echo -en '\001\033[01;32m\002')
UNDERLINE=$(echo -en '\001\033[01;4m\002')
GREEN_ARROW="${GREEN}->${RESTORE}"

echo "${GREEN_ARROW} ${UNDERLINE}Building command-line tools${RESTORE}"
./build.sh || {
  echo -e "${RED}FAILED: Couldn't build tools${RESTORE}"
  exit 1
}
echo -e "${GREEN}SUCCESS:${RESTORE} All tools compiled."
echo; echo

echo "${GREEN_ARROW} ${UNDERLINE}Running unit tests${RESTORE}"
cc -Ofast -Wall -pedantic -Wextra -Wno-missing-field-initializers \
    src/common.c \
    src/ngrams.c \
    src/readall.c \
    test/test-ngrams.c \
    -o test/test-ngrams \
&& ./test/test-ngrams
echo; echo

echo -e "${GREEN_ARROW} ${UNDERLINE}Running integration tests${RESTORE}"
echo -e "Test full tally on story3.txt"
./text-to-ngrams -n2 test/fixtures/story3.txt \
  | ./tally-lines -c \
  | sort -t$'\t' -bgr \
  | diff -b test/fixtures/story3.2grams.tallied - \
  || {
    echo -e "\x1B[31mFAILED: tally results not as expected\x1B[0m"
    exit 1
  }
echo -e "Test large file bom.4grams"
./tally-lines -c test/fixtures/bom.4grams \
  | sort -t$'\t' -bgr \
  | diff -qw test/fixtures/bom.4grams.tallied - \
  || {
    echo -e "\x1B[31mFAILED: tally results for large file not as expected\x1B[0m"
    echo "Run the following to see diff:"
    echo "  ./tally-lines -c test/fixtures/bom.4grams | sort -bgr | diff -w test/fixtures/bom.tallied.4grams -"
    exit 1
  }
echo -e "${GREEN}SUCCESS:${RESTORE} All integration tests have passed."
echo; echo
