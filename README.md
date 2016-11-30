Simple HTTP apps that respond with "200 OK" and a body of "Hello, World!" on
every request.

The test is to see how many req/s each implementation can handle.

Important note: None of the implementations make an external attempt to use
threads or multiple processes to increase performance. Only the performance of
what comes with the HTTP/networking implementation out of the box is tested.

# Building and running tests

Rust (Iron):

    $ cargo build --release
    $ ./target/release/irontest

Node.js:

    $ node test.js

Ruby:

    $ gem install puma
    $ puma -b tcp://[::]:3002 test.ru

C++ with `http-parser`:

    $ make REALM_CORE_PREFIX=<path> REALM_SYNC_PREFIX=<path>
    # Run with ASIO networking:
    $ ./irontest-asio
    # Run with Realm networking:
    $ export LD_LIBRARY_PATH=$REALM_CORE_PREFIX/src/realm:$REALM_SYNC_PREFIX/src/realm
    $ ./irontest-cpp

### Invoke `wrk`

    wrk -c <connections> http://[::1]:<port>

Test client runs for 10 seconds on 2 threads by default.

# Results

Tested versions:

- rustc 1.13, iron 0.4
- Node.js: 6.9.1
- Ruby MRI 2.3.1p112, Puma 3.6.2
- GCC 6.2.0, ASIO 1.10.6

Test system:

- Linux Mint 18 with Linux kernel 4.4.0 (64-bit)
- Intel Core i7-4790K CPU @ 4.00GHz
- 32 GiB RAM

Column header indicates number of concurrent connections.

|                | c=10    | c=100   | c=1000  |
|----------------|--------:|--------:|--------:|
| Rust           | 355,718 | 389,781 | 398,732 |
| Node.js        |  36,217 |  35,765 |  32,426 |
| Ruby + Puma    |  37,273 |  35,878 |  36,230 |
| C++ (ASIO) ^*  | 313,026 | 327,131 | 306,131 |
| C++ (Realm)    | 342,247 | 374,600 | 312,344 |

^*) Note: With `c=1000`, 22 requests timed out.

