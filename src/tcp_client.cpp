#include "tcp_client.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>

#include "client_connection.hpp"
#include "socket.hpp"
#include "utils.hpp"

TcpClient::TcpClient(std::string host, std::string port)
    : host_{host}, port_{port}, clientSocket_(-1) {}

ClientConnection TcpClient::connect() {
  auto addr{resolveAddress(port_, host_)};

  for (auto p{addr.get()}; p != NULL; p = p->ai_next) {
    if (tryConnect(p)) {
      return ClientConnection{std::move(clientSocket_)};
    }
  }

  throw std::runtime_error("Failed to connect to host");
}

bool TcpClient::tryConnect(addrinfo *p) {
  Socket s{socket(p->ai_family, p->ai_socktype, p->ai_protocol)};

  if (s.fd() == -1)
    return false;

  if (::connect(s.fd(), p->ai_addr, p->ai_addrlen) == -1)
    return false;

  clientSocket_ = std::move(s);
  return true;
}
