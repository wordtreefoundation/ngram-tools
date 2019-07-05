#include <inttypes.h>
#include "acutest.h"
#include "../src/ngrams.h"
#include "../src/common.h"

#define STORY_PATH "test/fixtures/story.txt"

int DEBUG = false;
int VERBOSE = false;

void check_each_story_2gram(char* start_ptr, size_t len, size_t index)
{
  const char* expected_results[] = {
    "this is", "is the", "the story", "story of", "of a", "a cat", "cat named",
    "named shebabwe", "this is", "is also", "also a", "a test", "test story"
  };
  const size_t expected_results_len = 13;
  
  TEST_CHECK(index < expected_results_len);
  TEST_CHECK_(
    memcmp(start_ptr, expected_results[index], len) == 0,
    "expected \"%s\" at iter %d", expected_results[index], index
  );
}

void check_each_story_3gram(char* start_ptr, size_t len, size_t index)
{
  const char* expected_results[] = {
    "this is the", "is the story", "the story of", "story of a", "of a cat",
    "a cat named", "cat named shebabwe", "this is also", "is also a",
    "also a test", "a test story"
  };
  const size_t expected_results_len = 11;
  
  TEST_CHECK(index < expected_results_len);
  TEST_CHECK_(
    memcmp(start_ptr, expected_results[index], len) == 0,
    "expected \"%s\" at iter %d", expected_results[index], index
  );
}
void test_for_each_ngram_of_file(void)
{
  FILE* input_file = open_file(STORY_PATH);
  for_each_ngram_of_file(input_file, 2, check_each_story_2gram);
  
  rewind(input_file);

  for_each_ngram_of_file(input_file, 3, check_each_story_3gram);
  fclose(input_file);
}

TEST_LIST = {
	{ "for_each_ngram_of_file", test_for_each_ngram_of_file },
	{ 0 }
};
