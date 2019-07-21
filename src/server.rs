use std::collections::HashMap;
use std::sync::Mutex;

extern crate tokio;
use tokio::prelude::*;
use tokio::{spawn, run};

extern crate serde;
#[macro_use]
extern crate serde_derive;

extern crate daemon_engine;
use daemon_engine::{Server, JsonCodec};

mod common;
use common::{Request, Response};

pub fn server(addr) {
    let server = future::lazy(move || {
        let mut s = UnixServer::<JsonCodec<Response, Request>>::new(&addr).unwrap();
        let m = Mutex::new(HashMap::<String, String>::new());

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
                            r.send(Response::Value(v.to_string()))
                        },
                        None => {
                            println!("Requested key: '{}' no value found", k);
                            r.send(Response::None)
                        },
                    },
                    Request::Set(k, v) => {
                        println!("Set key: '{}' value: '{}'", k, v);
                        m.lock().unwrap().insert(k, v.clone());
                        r.send(Response::Value(v.to_string()))
                    }
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
