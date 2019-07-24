use std::sync::{Arc, RwLock};

use varlink::{Connection};
use super::org_wordtree_ngrams;
use super::org_wordtree_ngrams::{VarlinkClientInterface};
use super::common::{Tally};

type Result<T> = std::result::Result<T, Box<std::error::Error>>;

pub fn run_client(addr: &str, tally: Arc<RwLock<Tally>>) -> Result<()> {
    let conn = Connection::with_address(&addr).unwrap();
    let mut iface = org_wordtree_ngrams::VarlinkClient::new(conn);

    let mut score: f64 = 0.0;

    let tally = tally.read().unwrap();
    for (key, value) in tally.iter() {
        let reply = iface.lookup(key.into()).call()?;
        if reply.tally > 0 {
            score += (*value as f64) / (reply.tally as f64);
        }
        // eprintln!("{}: {:?} of {}", key, value, reply.tally);
    }
    println!("Score: {:.*}", 9, score);

    Ok(())
}