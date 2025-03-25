#pragma once
#ifndef _CT_SOCKET_H_
#define _CT_SOCKET_H_F

#include <asio/asio.hpp>
#include <fmt/format.h>
#include <system_error>

int client(const std::string& raw_ip_address = "127.0.0.1", const uint16_t port_num = 3333);
void server(const uint16_t port_num = 3333);
int ctTcpSocket();
int ctAptSocket(const uint16_t port_num = 3333);
int bdAptSocket(const uint16_t port_num = 3333);
int connToEnd(const std::string& raw_ip_address = "127.0.0.1", const uint16_t port_num = 3333);
int dnscoonToEnd(const std::string& host, const std::string& port_num);
int aptNewSocket(const uint16_t port_num = 3333, const uint16_t BACKLOG_SIZE = 30);

#endif
