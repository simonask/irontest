#include <realm/util/network.hpp>
#include "http-parser/http_parser.h"

#include <iostream>

using namespace realm;
using namespace realm::util;

struct Server {
    network::Service service;
    network::Acceptor acceptor;
    network::Socket client;
    network::Endpoint endpoint;

    Server() : acceptor(service), client(service) {}

    void start(std::string listen_addr, std::string listen_port) {
        network::Resolver resolver(service);
        network::Resolver::Query query(listen_addr, listen_port, network::Resolver::Query::passive | network::Resolver::Query::address_configured);
        network::Endpoint::List endpoints;
        resolver.resolve(query, endpoints);
        network::Acceptor acceptor(service);
        auto i = endpoints.begin(); // FIXME: Always listens on first endpoint
        std::error_code ec;
        for (; i != endpoints.end(); ++i) {
            acceptor.open(i->protocol(), ec);
            if (!ec) {
                acceptor.set_option(network::SocketBase::reuse_address(true), ec);
                if (!ec) {
                    acceptor.bind(*i, ec);
                    if (!ec) {
                        break;
                    }
                }
                acceptor.close();
            }
        }
        if (ec) {
            std::cerr << "Error binding acceptor: " << ec << "\n";
            return;
        }
        endpoint = *i;
        acceptor.listen();

        std::cout << "Listening on port " << acceptor.local_endpoint().port() << "...\n";
        service.post([]() { std::cout << "Hello :)\n"; });
        service.run();
        initiate_accept();
        std::cout << "run() returned\n";
    }

    void initiate_accept() {
        acceptor.async_accept(client, endpoint, [this](auto ec) {
            if (ec == util::error::operation_aborted) {
                std::cerr << "aborted\n";
                return;
            }
            std::cout << "accepted\n";
            client.close();
            this->initiate_accept();
        });
    }
};

int main(int, char**) {
    Server server;
    server.start("127.0.0.1", "3004");
}

