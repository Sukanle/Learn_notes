#include <asio/asio.hpp>
#include <fmt/core.h>
#include <stdio.h>

constexpr uint32_t MAX_LENGTH = 1024;

int main() {
    try {
        asio::io_context iocc;
        asio::ip::tcp::socket sock(iocc);
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string("127.0.0.1"),
                                   3333);
        asio::error_code ec = asio::error::host_not_found;
        sock.connect(ep, ec);
        if (ec) {
            fmt::print(stderr, "Error code by {}.\nError message: {}",
                       ec.value(), ec.message());
            return ec.value();
        }
        char request[MAX_LENGTH];
        fputs("Enter message: ", stdout);
        fgets(request, MAX_LENGTH, stdin);
        size_t request_length = strlen(request);
        asio::write(sock, asio::buffer(request, request_length));

        char reply[MAX_LENGTH];
        size_t reply_length =
            asio::read(sock, asio::buffer(reply, request_length));
        fmt::print(stdin, "Reply is: {}\n",
                   std::string_view(reply, reply_length));
    } catch (const std::exception& e) {
        fmt::print(stderr, "Exception: {}\n", e.what());
    }

    return 0;
}
