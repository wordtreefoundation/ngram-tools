use std::sync::{Arc, RwLock};

use varlink::{VarlinkService};
use super::org_wordtree_ngrams;
use super::org_wordtree_ngrams::{VarlinkInterface, Call_Ping, Call_Lookup, Call_LookupAll};
use super::common::{Tally};

struct WordtreeNgrams {
    tally: Arc<RwLock<Tally>>
}

impl VarlinkInterface for WordtreeNgrams {
    fn ping(&self, call: &mut Call_Ping, ping: String) -> varlink::Result<()> {
        println!("Ping received.");
        call.reply(ping)
    }

    fn lookup(&self, call: &mut Call_Lookup, ngram: String) -> varlink::Result<()> {
        let tally = self.tally.read().unwrap();
        let count = match tally.get(&ngram) {
            Some(value) => *value,
            None => 0
        };
        println!("Lookup called ({}): responding with {}.", ngram, count);
        call.reply(count)
    }

    fn lookup_all(&self, call: &mut Call_LookupAll, ngrams: Vec<String>) -> varlink::Result<()> {
        let tally = self.tally.read().unwrap();
        let values: Vec<i64> = ngrams.iter().map(|ngram| match tally.get(ngram) {
            Some(value) => *value,
            None => 0
        }).collect();
        println!("LookupAll called: responding with {} tallies.", values.len());
        call.reply(values)
    }
}

pub fn run_server(address: &str, tally: Arc<RwLock<Tally>>) -> varlink::Result<()> {
    let wn = WordtreeNgrams { tally };
    let myinterface = org_wordtree_ngrams::new(Box::new(wn));
    let service = VarlinkService::new(
        "org.wordtree",
        "ngrams",
        "0.1",
        "http://wordtree.org",
        vec![Box::new(myinterface)],
    );
    varlink::listen(
        service,
        &address,
        &varlink::ListenConfig {
            initial_worker_threads: 1,
            max_worker_threads: 10,
            idle_timeout: 0,
            ..Default::default()
        },
    )?;

    Ok(())
}
