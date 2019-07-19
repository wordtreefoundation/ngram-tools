#[allow(unused_imports)]
#[macro_use]
extern crate gumdrop;

use gumdrop::Options;
use std::io;
use std::io::Read;
use std::str;

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
    let opts = GramsOptions::parse_args_default_or_exit();

    let mut buffer: Vec<u8> = Vec::new();

    if opts.files.len() == 0 {
        // Read from STDIN
        io::stdin().read_to_end(&mut buffer).unwrap();
        normalize_ascii(&mut buffer);

        let s = match str::from_utf8(&buffer) {
            Ok(v) => v,
            Err(e) => panic!("Invalid UTF-8 sequence: {}", e),
        };
        println!("{}", s);
    }

    // println!("{:#?}", opts);
}

fn normalize_ascii(text: &mut Vec<u8>) {
    // let mut norm = String::with_capacity(text.len());
    // let mut write_char: u8;
    // let iter = &text.into_iter();
    // let iter = &text.iter_mut();
    // while let Some(&mut this_char) = iter.next() {
    for this_char in text.iter_mut() {
        let intermediate_char: u8 = match this_char {
            b'A'...b'Z' => *this_char + 32,
            b' ' | b'(' | b')' => b' ',
            b'!' | b'?' => b'.',
            _ => *this_char,
        };

        *this_char = intermediate_char;

        // if next_char == '-' {
        //     join_lines = true;
        // } else if join_lines && next_char == ' ' {
        //     // ignore whitespace after a dash (i.e. including newlines, which is the
        //     // most common case because words that are broken by syllables are dashed)
        // } else if c == '.' && !just_added_period {

        // }

        // norm.push(c.to_ascii_lowercase());
    }
    // return norm;
}
