#[allow(unused_imports)]

#[macro_use]
extern crate gumdrop;

// Use stdout from grep_cli because io::stdout() is slower (due to forced line buffering)
extern crate grep_cli;
extern crate termcolor;

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

fn sliding_window<F>(text: &Vec<u8>, number_words: usize, mut action: F)
    where F: FnMut(&[&[u8]]) {
    for sentence in text.split(|c| c == &b'.') {
        if sentence.len() > 0 {
            let words: Vec<&[u8]> = sentence.split(|c| c == &b' ').collect();
            for ngram in words.windows(number_words) {
                action(ngram);
            }
        }
    }
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

        let mut stdout = grep_cli::stdout(termcolor::ColorChoice::Never);
        sliding_window(&result, number, move |ngram| {
            let mut first_word_written = false;
            for word in ngram {
                if first_word_written {
                    stdout.write(&[b' ']).unwrap();
                } else {
                    first_word_written = true;
                }
                stdout.write(word).unwrap();
            }
            stdout.write(&[b'\n']).unwrap();
        })
    }
}
