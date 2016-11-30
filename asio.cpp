#include <asio.hpp>
#include "http-parser/http_parser.h"

#include <iostream>
#include <vector>
#include <memory>

using namespace asio;
using asio::ip::tcp;

struct Server {
    static constexpr size_t buffer_size = 1024;

    io_service service;
    tcp::acceptor acceptor;
    tcp::socket client;
    tcp::endpoint endpoint;

    struct Client {
        Server& server;
        tcp::socket socket;
        std::unique_ptr<char[]> recv_buffer;
        std::string send_buffer;
        bool responding = false;
        http_parser parser;
        http_parser_settings settings;

        explicit Client(Server& server);
        void keep_reading();
        void close();
        void on_message_complete();
        void on_headers_complete();
        void on_url(const char* url, size_t len);

        static int on_message_complete(http_parser*);
        static int on_headers_complete(http_parser*);
        static int on_url(http_parser*, const char*, size_t);
    };

    std::unique_ptr<Client> next_client;
    std::vector<std::unique_ptr<Client>> clients;

    Server() : acceptor(service, tcp::endpoint(tcp::v6(), 3004)), client(service) {
    }

    void start() {
        std::cout << "Listening on port " << acceptor.local_endpoint().port() << "...\n";
        acceptor.listen(10);
        initiate_accept();
        service.run();
        std::cout << "run() returned\n";
    }

    void initiate_accept() {
        next_client = std::make_unique<Client>(*this);
        acceptor.async_accept(next_client->socket, [&](auto ec) {
            if (ec == asio::error::operation_aborted) {
                std::cerr << "aborted\n";
                return;
            }
            else if (ec) {
                std::cerr << "accept() error: " << ec.message() << "\n";
                return;
            }
            next_client->keep_reading();
            this->clients.push_back(std::move(next_client));
            this->initiate_accept();
        });
    }
};

Server::Client::Client(Server& server) : server(server), socket(server.service), recv_buffer(new char[buffer_size]) {
    http_parser_init(&parser, HTTP_REQUEST);
    parser.data = this;
    http_parser_settings_init(&settings);
    settings.on_message_complete = &Client::on_message_complete;
    settings.on_url = &Client::on_url;
    settings.on_headers_complete = &Client::on_headers_complete;
}

void Server::Client::keep_reading() {
    socket.async_read_some(asio::buffer(recv_buffer.get(), buffer_size), [this](std::error_code ec, size_t len) {
        if (ec == asio::error::operation_aborted) {
            close();
            return;
        }
        if (ec == asio::error::eof) {
            http_parser_execute(&parser, &settings, nullptr, 0);
            close();
            return;
        }
        if (!ec) {
            http_parser_execute(&parser, &settings, recv_buffer.get(), len);
            if (!responding)
                keep_reading();
        }
        else {
            std::cerr << "socket error: " << ec.message() << "\n";
            close();
        }
    });
}

void Server::Client::on_message_complete() {
    responding = true;
    const char body[] = "Hello, World!";
    std::stringstream ss;
    ss << "HTTP/1.1 200 OK\r\n"
        "Server: Realm\r\n"
        "Content-Length: " << std::strlen(body) << "\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n" << body;
    send_buffer = ss.str();
    asio::async_write(socket, asio::buffer(send_buffer), [this](std::error_code ec, size_t len) {
        responding = false;
        if (ec == asio::error::operation_aborted) {
            close();
            return;
        }
        if (ec) {
            std::cerr << "socket error (send): " << ec.message() << "\n";
        }
        if (http_should_keep_alive(&parser)) {
            keep_reading();
        }
        else {
            close();
        }
    });
}

void Server::Client::on_headers_complete() {
    // std::cout << "on_headers_complete()\n";
}

void Server::Client::on_url(const char* url, size_t len) {
    // std::cout << "on_url: " << std::string(url, len) << "\n";
}

int Server::Client::on_message_complete(http_parser* parser) {
    static_cast<Client*>(parser->data)->on_message_complete();
    return 0;
}

int Server::Client::on_headers_complete(http_parser* parser) {
    static_cast<Client*>(parser->data)->on_headers_complete();
    return 0;
}

int Server::Client::on_url(http_parser* parser, const char* url, size_t len) {
    static_cast<Client*>(parser->data)->on_url(url, len);
    return 0;
}

void Server::Client::close() {
    server.service.post([this]() {
        Server& server = this->server;
        auto it = std::find_if(begin(server.clients), end(server.clients), [this](auto& a) { return a.get() == this; });
        if (it != end(server.clients)) {
            server.clients.erase(it); // suicide
        }
    });
}

int main(int, char**) {
    Server server;
    server.start();
}

