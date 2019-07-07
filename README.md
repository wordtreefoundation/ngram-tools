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

### reduce.sh

This shell script uses `awk` to take a stream of ngrams and their counts as input, sum them, and produce a stream of ngrams and their summed counts as output.

For example, with these two files as input:

`file1.4grams.txt`:

```
      5 hello this is a
      3 test to see if
      1 it works or not
```

`file2.4grams.txt`:

```
      3 test to see if
      1 it works or not
      1 and maybe it will
```

We can reduce them as follows:

```
cat file1.4grams.txt file2.4grams.txt | ./reduce.sh
```

And get output like this:

```
      6 test to see if
      2 it works or not
      1 and maybe it will
      5 hello this is a
```

(Note that the contents of both files are combined, and the `test to see if` 4gram counts have been summed as 3+3=6).

### tally-ngrams

You can count up ngrams in a file and add them to an ongoing, persistent tally (database) via:

```
./tally-ngrams baseline.tkvdb <book.txt>
```

Where `<book.txt>` is any text file (e.g. a book from archive.org), and `baseline.tkvdb` is the name of a new [tkvdb](https://github.com/vmxdev/tkvdb) file. You can use any name you want for the database file if it's a new database, and the file will be created for you.

