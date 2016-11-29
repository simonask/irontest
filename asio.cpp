#include <asio.hpp>
#include "http-parser/http_parser.h"

#include <iostream>

using namespace asio;
using namespace asio::ip;

struct Server {
    static constexpr size_t buffer_size = 1024;

    io_service service;
    tcp::acceptor acceptor;
    tcp::socket client;
    tcp::endpoint endpoint;
    std::unique_ptr<char[]> buffer;
    http_parser parser;
    http_parser_settings settings;
    bool responding = false;
    std::string response;

    Server() : acceptor(service, tcp::endpoint(tcp::v6(), 3004)), client(service) {
        http_parser_settings_init(&settings);
        settings.on_message_complete = &Server::on_message_complete;
    }

    void start() {
        std::cout << "Listening on port " << acceptor.local_endpoint().port() << "...\n";
        buffer.reset(new char[buffer_size]);
        acceptor.listen(10);
        initiate_accept();
        service.run();
        std::cout << "run() returned\n";
    }

    void initiate_accept() {
        acceptor.async_accept(client, endpoint, [this](auto ec) {
            if (ec == asio::error::operation_aborted) {
                std::cerr << "aborted\n";
                return;
            }
            http_parser_init(&parser, HTTP_REQUEST);
            parser.data = this;
            responding = true;
            keep_reading();
        });
    }

    void keep_reading() {
        client.async_read_some(asio::buffer(buffer.get(), buffer_size), [this](auto ec, size_t length) {
            if (length != 0) {
                http_parser_execute(&parser, &settings, buffer.get(), length);
            }
            if (ec == asio::error::eof) {
                http_parser_execute(&parser, &settings, nullptr, 0);
                return;
            }
            else if (ec) {
                throw std::runtime_error("Socket error");
            }
            else if (!responding) {
                keep_reading();
            }
        });
    }

    void message_complete() {
        responding = true;
        std::string body = "Hello, World!";
        std::stringstream rs;
        rs << "HTTP/1.1 200 OK\r\n"
                    "Server: Realm+ASIO\r\n"
                    "Content-Length: " << body.size() << "\r\n"
                    "Content-Type: text/plain\r\n"
                    "\r\n";
        rs << body << "\r\n";
        response = rs.str();
        client.async_write_some(asio::buffer(response), [this](auto ec, size_t) {
            if (ec) {
                std::cerr << "error: " << ec << "\n";
            }
            client.close();
            initiate_accept();
        });
    }

    static int on_message_complete(http_parser* parser) {
        Server* server = static_cast<Server*>(parser->data);
        server->message_complete();
        return 0;
    }
};

int main(int, char**) {
    Server server;
    server.start();
}

