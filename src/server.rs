use std::collections::HashMap;
use std::sync::Mutex;

extern crate tokio;
use tokio::prelude::*;
use tokio::{spawn, run};

extern crate daemon_engine;
use daemon_engine::{JsonCodec, UnixServer};

use super::common::{Request, Response};

pub fn server(addr: String, tally: &'static HashMap<String, u32>) {
    let server = future::lazy(move || {
        let codec = JsonCodec::new();
        let mut s = UnixServer::<JsonCodec<Response, Request>>::new(&addr, codec).unwrap();
        let m = Mutex::new(tally);

        let server_handle = s
            .incoming()
            .unwrap()
            .for_each(move |r| {
                println!("Request: {:?}", r.data());
                let data = r.data();
                match data {
                    Request::Get(k) => match m.lock().unwrap().get(&k) {
                        Some(v) => {
                            println!("Requested key: '{}' value: '{}", k, v);
                            r.send(Response::Value(*v))
                        },
                        None => {
                            println!("Requested key: '{}' no value found", k);
                            r.send(Response::None)
                        },
                    },
                }.wait()
                .unwrap();

                Ok(())
            }).map_err(|_e| ());
        spawn(server_handle);
        Ok(())
    });

    run(server);

    println!("Done!");
}
