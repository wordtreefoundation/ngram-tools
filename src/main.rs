
#[allow(unused_imports)]
#[macro_use]
extern crate gumdrop;
extern crate serde;

use gumdrop::Options;

use std::fs::File;
use std::sync::{Arc, RwLock};
use std::collections::HashMap;
use std::io;
use std::io::ErrorKind;
use std::io::Read;

mod pipeline;
mod server;
mod client;
mod common;
mod org_wordtree_ngrams;

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

    #[options(help = "read input as tallied n-gram, rather than raw text")]
    tallied_input: bool,

    #[options(no_short, help = "server mode: serve tallied results when requested")]
    server: bool,

    #[options(no_short, help = "client mode: request tallied results")]
    client: bool,

    #[options(no_short, help = "print normalized ascii and exit")]
    normalize: bool,

    #[options(no_short, help = "print sliding window of ngrams and exit")]
    window: bool,
}

impl Default for GramsOptions {
    fn default() -> GramsOptions {
        GramsOptions {
            files: Vec::new(),
            help: false,
            number: None,
            sort: None,
            tallied_input: false,
            server: false,
            client: false,
            normalize: false,
            window: false,
        }
    }
}

fn error(msg: String) -> Result<(), io::Error> {
    return Err(io::Error::new(ErrorKind::Other, msg));
}

fn main() -> Result<(), io::Error> {
    let opts: GramsOptions = GramsOptions::parse_args_default_or_exit();
    let addr = "unix:/tmp/ngramd.sock".to_string();

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
    
    if opts.server && opts.client {
        return error("Error: Can't be both a server and a client".to_string());
    } else if opts.server {
        println!("Server mode enabled ({})", addr);
    } else if opts.client {
        println!("Client mode enabled ({})", addr);
    }

    let mut buffer: Vec<u8> = Vec::new();
    let mut tally: common::Tally = HashMap::new();
    if opts.files.len() == 0 {
        println!("Waiting for STDIN...");
        // Read from STDIN
        io::stdin().read_to_end(&mut buffer)?;
        if opts.tallied_input { 
            pipeline::read_tallied_input(&buffer, number, &mut tally)?;
        } else {
            pipeline::text_pipeline(&buffer, number, &mut tally, opts.normalize, opts.window)?;
        }
    } else {
        // Read from files
        for filename in opts.files {
            let mut file = File::open(&filename).expect("Error opening File");
            buffer.clear();
            file.read_to_end(&mut buffer)?;
            if opts.tallied_input { 
                pipeline::read_tallied_input(&buffer, number, &mut tally)?;
            } else {
                pipeline::text_pipeline(&buffer, number, &mut tally, opts.normalize, opts.window)?;
            }
        }
    }

    if opts.server {
        eprintln!("  -> serving {} ngrams", tally.len());
        server::run_server(&addr, Arc::new(RwLock::new(tally))).expect("Unable to run server");
    } else if opts.client {
        eprintln!("  -> requesting ngrams");
        client::run_client(&addr, Arc::new(RwLock::new(tally))).expect("Unable to run client");
    } else {
        pipeline::print_tally(&tally, &sort)?;
    }

    Ok(())
}
