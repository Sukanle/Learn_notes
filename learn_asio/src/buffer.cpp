#include <asio/asio.hpp>
#include <fmt/format.h>
#include <optional>
// #include <functional>

auto constBuf(const std::string& data) -> std::vector<asio::const_buffer> {
    asio::const_buffer asio_conbuf(data.c_str(), data.length());
    std::vector<asio::const_buffer> buffer_sequence;
    buffer_sequence.push_back(asio_conbuf);
    return buffer_sequence;
}

auto bufStr(const std::string& data) -> asio::const_buffers_1 {
    return asio::const_buffers_1(asio::buffer(data));
}

auto bufArray(const size_t size) -> asio::mutable_buffers_1 {
    std::unique_ptr<char[]> buf = std::make_unique<char[]>(size);
    return asio::buffer(static_cast<void*>(buf.get()), size);
}

void writeSock(asio::ip::tcp::socket& sock, const char* data,
               size_t msg_size = std::string::npos) {
    size_t total_bytes = 0;
    if (msg_size == std::string::npos) msg_size = strlen(data);

    while (total_bytes != msg_size) {
        total_bytes += sock.write_some(
            asio::buffer(data + total_bytes, msg_size - total_bytes));
    }
}

template<bool isclog = false>
auto sendData(const char* msg, const char* raw_ip = "127.0.0.1",
              uint16_t port = 3333,
              size_t msg_size = std::string::npos) -> std::optional<int> {
    if (msg_size == std::string::npos) msg_size = strlen(msg);
    try {
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip),
                                   port);
        asio::io_context iocc;
        asio::ip::tcp::socket sock(iocc, ep.protocol());
        sock.connect(ep);
        if constexpr (!isclog)
            writeSock(sock, msg, msg_size);
        else
            return sock.send(asio::buffer(msg, msg_size)) <= 0
                       ? std::optional<int>(0)
                       : std::nullopt;
    } catch (std::system_error& e) {
        fmt::print(stderr, "Error code = {}\nMessgae: {}", e.code(), e.what());
        return std::optional<int>(e.code().value());
    }
    return std::nullopt;
}

auto readSock(asio::ip::tcp::socket& sock, char* data,
              size_t msg_size = std::string::npos)
    -> std::optional<asio::error_code> {
    size_t total_bytes = 0;
    if (msg_size == std::string::npos) msg_size = strlen(data);

    // char buf[msg_size];
    asio::error_code e;
    while (total_bytes != msg_size) {
        total_bytes += sock.read_some(
            asio::buffer(data + total_bytes, msg_size - total_bytes), e);
        if (e) return std::optional<asio::error_code>(e);
    }
    // 将buf写入到data
    // std::copy(buf, buf + msg_size, const_cast<char*>(data.data()));
    return std::nullopt;
}

template<bool isclog = false>
auto readData(char* msg, size_t msg_size, const char* raw_ip = "127.0.0.1",
              uint16_t port = 3333) -> std::optional<int> {
    if (msg_size == std::string::npos) msg_size = strlen(msg);
    try {
        asio::ip::tcp::endpoint ep(asio::ip::address::from_string(raw_ip),
                                   port);
        asio::io_context iocc;
        asio::ip::tcp::socket sock(iocc, ep.protocol());
        sock.connect(ep);
        if constexpr (!isclog)
            readSock(sock, msg, msg_size);
        else
            return sock.receive(asio::buffer(msg, msg_size)) <= 0
                       ? std::optional<int>(0)
                       : std::nullopt;
    } catch (std::system_error& e) {
        fmt::print(stderr, "Error code = {}\nMessgae: {}", e.code(), e.what());
        return std::optional<int>(e.code().value());
    }
    return std::nullopt;
}
