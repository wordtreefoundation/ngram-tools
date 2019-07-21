#[allow(unused_imports)]
#[macro_use]
extern crate gumdrop;

// Use stdout from grep_cli because io::stdout() is slower (due to forced line buffering)
extern crate grep_cli;
extern crate termcolor;

extern crate serde;
#[macro_use]
extern crate serde_derive;

use gumdrop::Options;

use std::fs::File;

use std::collections::HashMap;
use std::io;
use std::io::ErrorKind;
use std::io::Read;

mod pipeline;
mod common;

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

fn error(msg: String) -> Result<(), io::Error> {
    return Err(io::Error::new(ErrorKind::Other, msg));
}

fn main() -> Result<(), io::Error> {
    let opts: GramsOptions = GramsOptions::parse_args_default_or_exit();

    let number: usize = match opts.number {
        Some(n) => n as usize,
        None => 1,
    };

    let sort: Option<common::Sort> = match opts.sort.as_ref().map(|s| &s[..]) {
        Some("a") | Some("alphabetic") | Some("") => Some(common::Sort::Alphabetic),
        Some("n") | Some("numeric") => Some(common::Sort::Numeric),
        Some(invalid_sort) => {
            return error(format!(
                "Unable to sort by {}. Use 'a' for alphabetic or 'n' for numeric.",
                invalid_sort
            ));
        }
        None => None,
    };

    let mut buffer: Vec<u8> = Vec::new();
    let mut tally: HashMap<String, usize> = HashMap::new();
    if opts.files.len() == 0 {
        // Read from STDIN
        io::stdin().read_to_end(&mut buffer).unwrap();
        pipeline::text_pipeline(&buffer, number, &mut tally, opts.normalized, opts.windowed)?;
    } else {
        // Read from files
        for filename in opts.files {
            let mut file = File::open(&filename).expect("Error opening File");
            buffer.clear();
            file.read_to_end(&mut buffer).unwrap();
            pipeline::text_pipeline(&buffer, number, &mut tally, opts.normalized, opts.windowed)?;
        }
    }

    pipeline::print_tally(&tally, &sort)?;

    Ok(())
}
