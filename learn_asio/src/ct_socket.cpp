#include <asio/asio.hpp>
#include <fmt/format.h>
#include <optional>

auto client(const std::string& raw_ip_address,
            const uint16_t port_num) -> std::optional<int> {
    // const char* raw_ip_address{"127.0.0.1"};
    // const uint16_t port_num = 3333;

    asio::error_code ec;
    asio::ip::address ip_address =
        asio::ip::address::from_string(raw_ip_address);
    if (ec.value() != 0) {
        fmt::print(
            stderr,
            "Failed to parse the IP address. Error code = {}.\nMessgae: {}\n",
            ec.value(), ec.message());
        return std::optional<int>(ec.value());
    }

    asio::ip::tcp::endpoint ep(ip_address, port_num);

    return std::nullopt;
}

void server(const uint16_t port_num) {
    asio::ip::address ip_address(asio::ip::address_v4::any());
    asio::ip::tcp::endpoint ep(ip_address, port_num);
}

auto ctTcpSocket() -> std::optional<int> {
    // asio::io_service becomes asio::io_context at asio version 1.18.0
    // 上下文对象，用于管理asio库的所有I/O功能
    asio::io_context iocc;
    // asio::ip::tcp protocol = asio::ip::tcp::v4();
    asio::ip::tcp::socket sock(iocc);
    // std::error_code ec;
    // sock.open(protocol,ec);
    // if (ec.value() != 0) {
    //     fmt::print(
    //         stderr,
    //         "Failed to parse the IP address. Error code = {}.\nMessgae:
    //         {}\n", ec.value(), ec.message());
    //     return ec.value();
    // }

    return std::nullopt;
}

auto ctAptSocket(const uint16_t port_num) -> std::optional<int> {
    asio::io_context iocs;
    // asio::ip::tcp::acceptor acceptor(iocs);
    // asio::ip::tcp protocol = asio::ip::tcp::v4();
    // std::error_code ec;
    // if (ec.value() != 0) {
    //     fmt::print(stderr, "Failed to parse the IP address. Error code =
    //     {}.\nMessgae: {}\n",
    //                ec.value(), ec.message());
    //     return ec.value();
    // }
    asio::ip::tcp::acceptor ap(
        iocs, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port_num));

    return std::nullopt;
}

auto bdAptSocket(const uint16_t port_num) -> std::optional<int> {
    // const uint16_t port_num = 3333;
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
    asio::io_context iocs;
    asio::ip::tcp::acceptor ap(iocs, ep.protocol());
    asio::error_code ec;
    ap.bind(ep, ec);
    if (ec.value() != 0) {
        fmt::print(stderr,
                   "Failed to bind the acceptor socket. Error code = "
                   "{}.\nMessgae: {}\n",
                   ec.value(), ec.message());
        return std::optional<int>(ec.value());
    }
    return std::nullopt;
}

auto connToEnd(const std::string& raw_ip_address,
               const uint16_t port_num) -> std::optional<int> {
    // std::string raw_ip_address{"8.134.84.119"};
    // const uint16_t port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(
            asio::ip::address::from_string(raw_ip_address), port_num);
        asio::io_context iocs;
        asio::ip::tcp::socket sock(iocs, ep.protocol());
        sock.connect(ep);
    } catch (asio::system_error& e) {
        fmt::print(stderr, "Error occured! Error code = {}.\nMessage: {}\n",
                   e.code().value(), e.what());
        return std::optional<int>(e.code().value());
    }

    return std::nullopt;
}

// 通过域名连接，一般在C++中不建议使用域名（耗时），一般是在外部通过DNS解析后传入IP地址
auto dnscoonToEnd(const std::string& host,
                  const std::string& port_num) -> std::optional<int> {
    // std::string host = "llfc.club";
    // std::string port_num = "3333";
    asio::io_context iocs;
    asio::ip::tcp::resolver::query resolver_q(
        host, port_num, asio::ip::tcp::resolver::query::numeric_service);
    asio::ip::tcp::resolver resolver(iocs);
    try {
        auto it = resolver.resolve(resolver_q);
        asio::ip::tcp::socket sock(iocs);
        asio::connect(sock, it);
    } catch (asio::system_error& e) {
        fmt::print(stderr, "Error occured! Error code = {}.\nMessage: {}\n",
                   e.code().value(), e.what());
        return std::optional<int>(e.code().value());
    }

    return std::nullopt;
}

auto aptNewSocket(const uint16_t port_num,
                  const uint16_t BACKLOG_SIZE) -> std::optional<int> {
    asio::io_context iocs;
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);

    try {
        asio::ip::tcp::acceptor ap(iocs, ep.protocol());
        ap.bind(ep);
        ap.listen(BACKLOG_SIZE);
        asio::ip::tcp::socket sock(iocs);
        ap.accept(sock);
    } catch (asio::system_error& e) {
        fmt::print(stderr, "Error occured！Error code = {}.\nMessage: {}\n",
                   e.code().value(), e.what());
        return std::optional<int>(e.code().value());
    }

    return std::nullopt;
}
