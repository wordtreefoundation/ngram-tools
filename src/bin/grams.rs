#[allow(unused_imports)]
#[macro_use]
extern crate gumdrop;
// #[macro_use] extern crate log;

use gumdrop::Options;
use std::io;
use std::io::Read;
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
    let opts = GramsOptions::parse_args_default_or_exit();

    let mut buffer: Vec<u8> = Vec::new();

    if opts.files.len() == 0 {
        // Read from STDIN
        io::stdin().read_to_end(&mut buffer).unwrap();
        let result = normalize::normalize_ascii(&buffer);

        let s = match str::from_utf8(&result) {
            Ok(v) => v,
            Err(e) => panic!("Invalid UTF-8 sequence: {}", e),
        };
        println!("{}", s);
    }

    // println!("{:#?}", opts);
}
