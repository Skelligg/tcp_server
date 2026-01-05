#include <arpa/inet.h>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>
#include <system_error>

#include "client_connection.hpp"
#include "socket.hpp"
#include "tcp_server.hpp"
#include "utils.hpp"

TcpServer::TcpServer(std::string port, int backlog, sa_family_t ss_family)
    : port_{port}, backlog_{backlog}, ss_family_{ss_family}, serverSocket_{-1} {
}

void TcpServer::start() {
  auto addr{resolveAddress(port_)};

  for (auto p{addr.get()}; p != NULL; p = p->ai_next) {
    if (tryBindAndListen(p)) {
      return;
    }
  }

  throw std::runtime_error("Failed to bind to any address");
}

ClientConnection TcpServer::acceptConnection() {
  sockaddr_storage theirAddr;
  socklen_t sinSize{sizeof(theirAddr)};

  Socket s{accept(serverSocket_.fd(), (sockaddr *)&theirAddr, &sinSize)};
  if (s.fd() == -1)
    throw std::system_error(errno, std::system_category(), "accept");

  char theirAddrName[INET6_ADDRSTRLEN];

  inet_ntop(theirAddr.ss_family, get_in_addr((sockaddr *)&theirAddr),
            theirAddrName, sizeof theirAddrName);
  printf("server: got connection from %s\n", theirAddrName);

  return ClientConnection{std::move(s)};
}

bool TcpServer::tryBindAndListen(addrinfo *p) {
  Socket s{socket(p->ai_family, p->ai_socktype, p->ai_protocol)};

  if (s.fd() == -1)
    return false;

  int yes{1};
  if (setsockopt(s.fd(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    return false;

  if (bind(s.fd(), p->ai_addr, p->ai_addrlen) == -1)
    return false;

  if (listen(s.fd(), backlog_) == -1)
    return false;

  serverSocket_ = std::move(s);
  return true;
}
