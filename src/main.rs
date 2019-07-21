#[allow(unused_imports)]
#[macro_use]
extern crate gumdrop;

// Use stdout from grep_cli because io::stdout() is slower (due to forced line buffering)
extern crate grep_cli;
extern crate termcolor;

use gumdrop::Options;

use std::fs::File;

use std::collections::HashMap;
use std::io;
use std::io::ErrorKind;
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

    #[options(help = "sort tallied output: a=alphabetic, n=numeric")]
    sort: Option<String>,

    #[options(no_short, help = "print normalized ascii and exit")]
    normalized: bool,

    #[options(no_short, help = "print sliding window of ngrams and exit")]
    windowed: bool,
}

impl Default for GramsOptions {
    fn default() -> GramsOptions {
        GramsOptions {
            files: Vec::new(),
            help: false,
            number: None,
            normalized: false,
            windowed: false,
            sort: None,
        }
    }
}

fn sliding_window<F>(text: &Vec<u8>, number_words: usize, mut action: F)
where
    F: FnMut(&[&[u8]]),
{
    for sentence in text.split(|c| c == &b'.') {
        if sentence.len() > 0 {
            let words: Vec<&[u8]> = sentence.split(|c| c == &b' ').collect();
            for ngram in words.windows(number_words) {
                action(ngram);
            }
        }
    }
}

fn print_ngram(stdout: &mut grep_cli::StandardStream, ngram: &[&[u8]]) {
    let mut iter = ngram.iter();
    match iter.next() {
        Some(word) => {
            stdout.write(word).unwrap();
            ()
        }
        None => return,
    }
    while let Some(word) = iter.next() {
        stdout.write(&[b' ']).unwrap();
        stdout.write(word).unwrap();
    }
    stdout.write(&[b'\n']).unwrap();
}

fn text_pipeline(text: &Vec<u8>, window_size: usize, sort: &Option<Sort>, normalized_only: bool, windowed_only: bool) -> Result<(), io::Error> {
    let mut stdout = grep_cli::stdout(termcolor::ColorChoice::Never);

    let result = normalize::normalize_ascii(&text);

    if normalized_only {
        stdout.write(&result)?;
        return Ok(());
    }

    if windowed_only {
        sliding_window(&result, window_size, move |ngram| {
            print_ngram(&mut stdout, ngram)
        });
        return Ok(());
    }

    let mut tally: HashMap<String, usize> = HashMap::new();
    sliding_window(&result, window_size, |ngram| {
        let mut key: String = String::new();
        for (i, word) in ngram.iter().enumerate() {
            if i > 0 {
                key.push(' ');
            }
            key.push_str(str::from_utf8(word).unwrap());
        }
        let count = tally.entry(key).or_insert(0);
        *count += 1;
    });

    let mut pairs: Vec<(&String, &usize)>;
    match sort {
        Some(sort_order) => {
            pairs = tally.iter().collect();
            match sort_order {
                Sort::Alphabetic => pairs.sort_by(|a, b| a.0.cmp(b.0)),
                Sort::Numeric => pairs.sort_by(|a, b| b.1.cmp(a.1)),
            }

            for (key, value) in pairs {
                stdout.write(format!("{:9}\t{}\n", value, key).as_bytes())?;
            }
        }
        None => {
            for (key, value) in tally {
                stdout.write(format!("{:9}\t{}\n", value, key).as_bytes())?;
            }
        }
    }

    Ok(())
}

fn error(msg: String) -> Result<(), io::Error> {
    return Err(io::Error::new(ErrorKind::Other, msg));
}

enum Sort {
    Alphabetic,
    Numeric,
}

fn main() -> Result<(), io::Error> {
    let opts: GramsOptions = GramsOptions::parse_args_default_or_exit();

    let number: usize = match opts.number {
        Some(n) => n as usize,
        None => 1,
    };

    let sort: Option<Sort> = match opts.sort.as_ref().map(|s| &s[..]) {
        Some("a") | Some("alphabetic") | Some("") => Some(Sort::Alphabetic),
        Some("n") | Some("numeric") => Some(Sort::Numeric),
        Some(invalid_sort) => {
            return error(format!(
                "Unable to sort by {}. Use 'a' for alphabetic or 'n' for numeric.",
                invalid_sort
            ));
        }
        None => None,
    };

    let mut buffer: Vec<u8> = Vec::new();
    if opts.files.len() == 0 {
        // Read from STDIN
        io::stdin().read_to_end(&mut buffer).unwrap();
        text_pipeline(&buffer, number, &sort, opts.normalized, opts.windowed)?;
    } else {
        // Read from files
        for filename in opts.files {
            let mut file = File::open(&filename).expect("Error opening File");
            file.read_to_end(&mut buffer).unwrap();
            text_pipeline(&buffer, number, &sort, opts.normalized, opts.windowed)?;
        }
    }

    Ok(())
}
