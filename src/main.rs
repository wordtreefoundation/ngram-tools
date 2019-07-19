#[allow(unused_imports)]

#[macro_use]
extern crate gumdrop;

use gumdrop::Options;
use std::io;
use std::io::Read;
use std::io::Write;
use std::str;

mod normalize;

#[derive(Debug, Options)]
struct GramsOptions {
    #[options(free, help = "text files to read as n-grams")]
    files: Vec<String>,

    #[options(short = "h", help = "show help message")]
    help: bool,

    #[options(short = "n", help = "number of words per n-gram")]
    number: Option<i32>,
}

fn main() {
    let opts: GramsOptions = GramsOptions::parse_args_default_or_exit();

    let mut buffer: Vec<u8> = Vec::new();

    let number: usize = match opts.number {
        Some(n) => n as usize,
        None => 1
    };
    if opts.files.len() == 0 {
        // Read from STDIN
        io::stdin().read_to_end(&mut buffer).unwrap();
        let result = normalize::normalize_ascii(&buffer);

        let stdout = io::stdout();
        let mut out_handle = stdout.lock();
        for sentence in result.split(|c| c == &b'.') {
            if sentence.len() > 0 {
                let words: Vec<&[u8]> = sentence.split(|c| c == &b' ').collect();
                for ngram in words.windows(number) {
                    let mut first_word_written = false;
                    for word in ngram {
                        if first_word_written {
                            out_handle.write_all(&[b' ']).unwrap();
                        } else {
                            first_word_written = true;
                        }
                        out_handle.write_all(word).unwrap();
                    }
                    out_handle.write_all(&[b'\n']).unwrap();
                }
            }
        }
    }
}
