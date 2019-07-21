use std::collections::HashMap;

extern crate tokio;
use tokio::prelude::*;

extern crate tokio_uds;

extern crate daemon_engine;
use daemon_engine::{DaemonError, JsonCodec, UnixConnection};

use super::common::{Request, Response};

pub fn client(addr: String, tally: &HashMap<String, u32>) {
    // Create client connector
    let codec = JsonCodec::new();
    let client = UnixConnection::<JsonCodec<Request, Response>>::new(&addr, codec);
    let (tx, rx) = client.split();

    // match value {
    //     Some(value) => {
    //         println!("Set key: '{}'", key);
    //         tx.send(Request::Set(key, value.to_string()))
    //     }
    //     None => {
    //         println!("Get key: '{}'", key);
    //         tx.send(Request::Get(key))
    //     }
    // }.wait()
    // .unwrap();
    tx.send(Request::Get("came to pass that".to_string())).wait().unwrap();

    rx.map(|resp| -> Result<(), DaemonError> {
        println!("Response: {:?}", resp);
        Ok(())
    }).wait()
    .next();
}
