# ngram tools

Fast C-based command-line tools for processing text files, extracting ngrams, and making a statistical baseline (rough "language model").

## Installing

First, clone this repo with git, then install the `tkvdb` dependency with `git submodule`:

```
git submodule update --init
```

Finally, run the build script:

```
./build.sh
```

## Usage

### text-to-ngrams

The `text-to-ngrams` tool takes an ascii text file and outputs each ngram, one per line.

It takes a required value of `N` for the number of ngrams (e.g. `3` for 3-grams, `4` for 4-grams, etc.), and an optional filename. If no filename is given, STDIN is used.

Example:
```
$ ./text-to-ngrams 3 ../bomdb/bom.txt
Input File: ../bomdb/bom.txt
size (bytes): 1417363
i nephi having
nephi having been
having been born
been born of
born of goodly
of goodly parents
...
```

This tool is particularly useful in combination with unix `sort` and `uniq` commands. For example, here we can tally the number of 3-grams in the Book of Mormon:

```
$ ./text-to-ngrams 3 ../bomdb/bom.txt | sort | uniq -c | sort -bgr | head -30
Input File: ../bomdb/bom.txt
size (bytes): 1417363
  ngrams emitted: 253386
   1398 came to pass
   1396 it came to
   1333 to pass that
   1164 and it came
    481 of the lord
    444 the land of
    342 the people of
    290 of the land
...
```

### tally-ngrams

You can count up ngrams in a file and add them to an ongoing, persistent tally (database) via:

```
./tally-ngrams baseline.tkvdb <book.txt>
```

Where `<book.txt>` is any text file (e.g. a book from archive.org).

