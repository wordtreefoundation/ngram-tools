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

This will change, but for now, you can count up ngrams in a file and add them to an ongoing, persistent tally via:

```
./tally-ngrams baseline.tkvdb <book.txt>
```

Where `<book.txt>` is any text file (e.g. a book from archive.org).