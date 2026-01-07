#pragma once
#include "client_connection.hpp"
#include <netdb.h>
#include <string>

class TcpClient {
public:
  TcpClient(std::string host, std::string port);

  ClientConnection connect();

private:
  std::string host_;
  std::string port_;
  Socket clientSocket_;

  bool tryConnect(addrinfo *p);
  inline void *get_in_addr(sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
      auto *ipv4 = reinterpret_cast<sockaddr_in *>(sa);
      return &(ipv4->sin_addr);
    } else { // AF_INET6
      auto *ipv6 = reinterpret_cast<sockaddr_in6 *>(sa);
      return &(ipv6->sin6_addr);
    }
  }
};
