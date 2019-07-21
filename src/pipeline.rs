use std::io;
use std::io::Write;
use std::collections::HashMap;
use std::str;

mod normalize;
use super::common::{Sort};

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

pub fn text_pipeline(
    text: &Vec<u8>,
    window_size: usize,
    tally: &mut HashMap<String, usize>,
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
            print_ngram(&mut stdout, ngram)
        });
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
    });

    Ok(())
}

pub fn print_tally(tally: &HashMap<String, usize>, sort: &Option<Sort>) -> Result<(), io::Error> {
    let mut stdout = grep_cli::stdout(termcolor::ColorChoice::Never);

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

