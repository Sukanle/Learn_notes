#include <asio/asio.hpp>
#include <asio/ssl.hpp>
#include <iostream>
#include <type_traits>

void HttpRequest(std::string const &server, std::string const &path);
void HttpsRequest(std::string const &server, std::string const &path);
template<typename socket_t>
void HandleRequest(socket_t& socket,const std::string& server,
                   const std::string& path){
    std::string request = "GET "+path+"HTTP/1.1\r\n";
    request +="Host: "+server+"\r\n";
    request +="Accpet: */*\r\n";
    request +="Connection: close\r\n\r\n";

    asio::write(socket,asio::buffer(request));
    char buffer[1024]{};
    asio::error_code error;

    while(size_t len=socket.read_some(asio::buffer(buffer),error)){
std::cout.write(buffer,len);
    }
}

using asio::ip::tcp;

int main() {
    std::string const server = "example.com";
    std::string const path = "/";

    try {
        HttpsRequest(server, path);
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

void HttpRequest(std::string const &server, std::string const &path) {
    asio::io_context ioContext;

    tcp::resolver resolver(ioContext);
    auto endpoints = resolver.resolve(server, "http");

    tcp::socket socket(ioContext);

    asio::connect(socket, endpoints);

    HandleRequest(socket, server, path);
}
void HttpsRequest(std::string const &server, std::string const &path)
{
    asio::io_context ioContext;
    asio::ssl::context sslContext(asio::ssl::context::sslv23);
    sslContext.set_default_verify_paths();
    sslContext.set_verify_mode(asio::ssl::verify_peer);

    tcp::resolver resolver(ioContext);
    auto endpoints=resolver.resolve(server,"https");

    asio::ssl::stream<tcp::socket> socket(ioContext,sslContext);

    asio::connect(socket.lowest_layer(),endpoints);
    socket.handshake(asio::ssl::stream_base::client);
    HandleRequest(socket,server,path);
}
