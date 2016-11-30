extern crate iron;

use iron::prelude::*;

fn hello_world(_: &mut Request) -> IronResult<Response> {
    Ok(Response::with((iron::status::Ok, "Hello World!")))
}

fn main() {
    let chain = Chain::new(hello_world);
    let listen = ":::3000";
    println!("Listening at {}", listen);
    Iron::new(chain).http(listen).unwrap();
}
