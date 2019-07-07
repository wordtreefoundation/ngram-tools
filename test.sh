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
./text-to-ngrams -n 2 test/fixtures/story3.txt \
  | ./tally-lines -c \
  | sort -bgr \
  | diff -b test/fixtures/story3.2grams - \
  || {
    echo -e "\x1B[31mFAILED: tally results not as expected\x1B[0m"
    exit 1
  }
echo -e "${GREEN}SUCCESS:${RESTORE} All integration tests have passed."
echo; echo
