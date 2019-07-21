extern crate tokio;
use tokio::prelude::*;

extern crate tokio_uds;
use tokio_uds::UnixStream;

extern crate serde;
#[macro_use]
extern crate serde_derive;

extern crate daemon_engine;
use daemon_engine::{Connection, DaemonError, JsonCodec};

mod common;
use common::{Request, Response};

fn client(addr, key: String, value: Option<String>) {
    // Create client connector
    let client = UnixConnection::<JsonCodec<Request, Response>>::new(&addr);
    let (tx, rx) = client.split();

    match value {
        Some(value) => {
            println!("Set key: '{}'", key);
            tx.send(Request::Set(key, value.to_string()))
        }
        None => {
            println!("Get key: '{}'", key);
            tx.send(Request::Get(key))
        }
    }.wait()
    .unwrap();

    rx.map(|resp| -> Result<(), DaemonError> {
        println!("Response: {:?}", resp);
        Ok(())
    }).wait()
    .next();
}
