#pragma once

#include <string>
#include <sys/socket.h>

#include "client_connection.hpp"
#include "socket.hpp"
#include <netdb.h>
#include <unordered_map>

class TcpServer {
public:
  explicit TcpServer(std::string port, int backlog,
                     sa_family_t ss_family = AF_UNSPEC);
  void start();
  void run();

private:
  std::string port_;
  int backlog_;
  sa_family_t ss_family_;
  Socket serverSocket_;
  Socket epoll_;
  std::unordered_map<int, ClientConnection> clients_;

  void acceptConnection();
  void handleClientConnection(int fd, uint32_t events);
  void removeClient(int fd);

  bool tryBindAndListen(addrinfo *p);
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
