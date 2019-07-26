use std::io;
use std::io::Write;
use std::str;

// Use stdout from grep_cli because io::stdout() is slower (due to forced line buffering)
extern crate grep_cli;
extern crate termcolor;

mod normalize;
use super::common::{Sort, Tally};

fn sliding_window<F>(text: &Vec<u8>, number_words: usize, mut action: F) -> Result<(), io::Error>
where
    F: FnMut(&[&[u8]]) -> Result<(), io::Error>,
{
    for sentence in text.split(|c| c == &b'\n') {
        if sentence.len() > 0 {
            let words: Vec<&[u8]> = sentence.split(|c| c == &b' ').collect();
            for ngram in words.windows(number_words) {
                action(ngram)?;
            }
        }
    }

    Ok(())
}

fn print_ngram(stdout: &mut grep_cli::StandardStream, ngram: &[&[u8]]) -> Result<(), io::Error> {
    let mut iter = ngram.iter();
    match iter.next() {
        Some(word) => {
            stdout.write(word)?;
            ()
        }
        None => return Ok(()),
    }
    while let Some(word) = iter.next() {
        stdout.write(&[b' '])?;
        stdout.write(word)?;
    }
    stdout.write(&[b'\n'])?;

    Ok(())
}

pub fn text_pipeline(
    text: &Vec<u8>,
    window_size: usize,
    tally: &mut Tally,
    normalized_only: bool,
    windowed_only: bool,
) -> Result<(), io::Error> {
    let mut stdout = grep_cli::stdout(termcolor::ColorChoice::Never);

    let result = normalize::normalize_ascii(&text);

    if normalized_only {
        stdout.write(&result)?;
        return Ok(());
    }

    if windowed_only {
        sliding_window(&result, window_size, move |ngram| {
            print_ngram(&mut stdout, ngram)?;
            Ok(())
        })?;
        return Ok(());
    }

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

        Ok(())
    })?;

    Ok(())
}

fn eprint_utf8_fallback(text: &[u8]) {
    match str::from_utf8(text) {
        Ok(s) => eprintln!("{}", s.to_string()),
        Err(_) => eprintln!("{:?}", text),
    }
}

pub fn read_tallied_input(
    text: &Vec<u8>,
    window_size: usize,
    tally: &mut Tally) -> Result<(), io::Error> {

    for line in text.split(|c| c == &b'\n') {
        let pair: Vec<&[u8]> = line.split(|c| c == &b'\t').collect();
        if pair.len() == 2 {
            match str::from_utf8(pair[1]) {
                Ok(key) => {
                    match str::from_utf8(pair[0]) {
                        Ok(value) => {
                            match value.trim().parse::<u32>() {
                                Ok(v) => {
                                    let count = tally.entry(key.to_string()).or_insert(0);
                                    *count += v as i64;
                                }
                                Err(_e) => eprintln!("failed to parse tally for key {}: {}", key.to_string(), value.to_string())
                            }
                        }
                        Err(_e) => eprintln!("failed to read tally for key {}: {:?}", key.to_string(), pair[0])
                    }
                },
                Err(e) => {
                    eprintln!("failed to read ascii key: {:?}", e);
                    eprint_utf8_fallback(line);
                }
            }
        }
    }

    Ok(())
}

pub fn print_tally(tally: &Tally, sort: &Option<Sort>) -> Result<(), io::Error> {
    let mut stdout = grep_cli::stdout(termcolor::ColorChoice::Never);

    let mut pairs: Vec<(&String, &i64)>;
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

