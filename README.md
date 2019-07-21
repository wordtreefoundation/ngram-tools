# ngram tools

Fast rust-based command-line tools for processing text files, extracting ngrams, and making a statistical baseline (rough "language model").

- normalizes ascii text (lowercases; removes non-alphabetical text)
- takes windowed word slices as n-grams
- tallies the occurrences of n-grams

TODO:

- client/server architecture for fast look-up of in-memory n-gram tallies

## Installing

Install rust & cargo, then:

```
cargo build --release
```

## Usage

### ngrams

The `ngrams` tool takes an ascii text file and outputs tallied n-grams. The tally comes first and is space-padded, followed by a tab separator, and lastly the n-gram.

Input is taken to be STDIN if no files are given; otherwise, each given ascii file is processed in turn.

You can optionally specify the window size, i.e. the value of `n` (`--number`) in the `n`-grams (e.g. `3` for 3-grams, `4` for 4-grams, etc.): `ngrams -n4 [FILES]`.

You can also optionally request the output to be sorted with the `-s` (`--sort`) flag. Alphabetical sorting is given by `-sa` and numerical sorting is given with `-sn`: `ngrams -n4 -sn [FILES]`.

Example:

```
$ ./ngrams -n4 -sa ../bomdb/bom.txt
     1398	it came to pass
     1299	came to pass that
     1155	and it came to
      257	i say unto you
      234	to pass that the
      169	to pass that when
      159	now it came to
...
```

If more than one file is given on the command line, all of the results will be merged (accumulatd) in the output tally.

### Saving off and reloading results

If you send the tallied results to a file, you can load them back again with the `-t` (`--tallied-input`) flag:

```
# Redirect tallied, unsorted output to file `bom.4grams.tallied`:
$ ./ngrams -n4 ../bomdb/bom.txt >bom.4grams.tallied

# Re-read tallied data, then sort it and show the result:
$ ./ngrams -t -sn bom.4grams.tallied
     1398	it came to pass
     1299	came to pass that
     1155	and it came to
      257	i say unto you
      234	to pass that the
      169	to pass that when
      159	now it came to
...
```

This capability is particularly useful if you've crated a large aggregate file (i.e. baseline) that is the result of processing many files.
