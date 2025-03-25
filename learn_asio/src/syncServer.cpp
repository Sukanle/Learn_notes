#include <asio/asio.hpp>
#include <fmt/core.h>
#include <memory.h>
#include <optional>
#include <tbb/tbb.h>
// #include <set>

constexpr uint32_t MAX_LENGTH = 1024;

using socket_sptr = std::shared_ptr<asio::ip::tcp::socket>;
// std::set<std::shared_ptr<std::thread>> thread_set;

class tbb_server {
private:
    socket_sptr _sock = nullptr;
    std::optional<asio::error_code> _ec = std::nullopt;
    asio::system_error _se = asio::system_error();

public:
    tbb_server() noexcept = default;
    tbb_server(const tbb_server&) noexcept = default;
    tbb_server(tbb_server&&) noexcept = default;
    ~tbb_server() noexcept = default;
    tbb_server& operator=(const tbb_server&) noexcept = default;
    tbb_server& operator=(tbb_server&&) noexcept = default;
    void operator()(tbb::blocked_range<size_t>& range) {
        for (size_t i = range.begin(); i != range.end(); ++i) {
            try {
                _ec = session();
            } catch (const asio::system_error& e) {
                _se = e;
                return;
            }
        }
    }
    static void server(asio::io_context& iocs, uint16_t port,
                       tbb_server& self) {
        fmt::print(stdout, "Server is running on port: {}\n", port);
        asio::ip::tcp::acceptor acceptor(
            iocs, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
        while (true) {
            self._sock = std::make_shared<asio::ip::tcp::socket>(iocs);
            acceptor.accept(*self._sock);
            tbb::parallel_for(
                tbb::blocked_range<size_t>(0, 4),
                [&self](tbb::blocked_range<size_t>& range) { self(range); });
        }
    }
    auto session() -> std::optional<asio::error_code> {
        try {
            char data[MAX_LENGTH];
            asio::io_context iocs;
            asio::error_code ec;
            size_t length =
                _sock->read_some(asio::buffer(data, MAX_LENGTH), ec);
            if (ec == asio::error::eof) {
                fmt::print(stderr, "Connection closed by peer\n");
                return ec;
            } else if (ec)
                throw asio::system_error(ec);
            fmt::print(stdout, "Receive from clinet: {}\n",
                       _sock->remote_endpoint().address().to_string());
            fmt::print(stdout, "Receive message: {}\n", data);
            _sock->write_some(asio::buffer(data, length));
        } catch (const std::exception& e) {
            fmt::print(stderr, "Exception: {}\n", e.what());
        }
        return std::nullopt;
    }
    auto get_ec() const noexcept -> std::optional<asio::error_code> {
        return _ec;
    }
    auto get_se() const noexcept -> asio::system_error { return _se; }
    auto get_sock() const noexcept -> socket_sptr { return _sock; }
};

int main() {
    tbb_server server;
    asio::io_context iocs;
    server.server(iocs, 3333, server);
#define ec_value() get_ec().value()
    if (server.get_ec())
        fmt::print(stderr, "Error code: {}.\nError message: {}\n",
                   server.ec_value().value(), server.ec_value().message());
    if (server.get_se().code())
        fmt::print(stderr, "System error: {}\n", server.get_se().what());
}
