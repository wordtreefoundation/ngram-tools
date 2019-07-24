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
    let keys1: Vec<String> = tally.iter().map(|(k, _v)| k.clone() ).collect();
    let keys2: Vec<String> = tally.iter().map(|(k, _v)| k.clone() ).collect();
    let reply = iface.lookup_all(keys1).call()?;

    for (key, baseline) in keys2.iter().zip(reply.tallies.iter()) {
        let baseline = *baseline as f64;
        if baseline > 0.0 {
            let value = match tally.get(key) {
                Some(v) => *v,
                None => 0
            };
            score += (value as f64) / (baseline as f64)
        }
    }
    
    println!("Score: {:.*}", 9, score);

    Ok(())
}