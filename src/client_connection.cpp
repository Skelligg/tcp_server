#include "client_connection.hpp"
#include "socket.hpp"
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <utility>
#include <vector>

ClientConnection::ClientConnection(Socket s) : s_{std::move(s)} {}

void ClientConnection::recvSome() {
  std::vector<char> buffer(1024);
  auto bytesReceived{recv(s_.fd(), buffer.data(), buffer.size(), 0)};
  if (bytesReceived > 0) {
    std::string received{buffer.data(),
                         static_cast<std::string::size_type>(bytesReceived)};
    std::cout << received << '\n';
  } else if (bytesReceived == 0) {
    std::cout << "client: closed connection" << '\n';
  } else {
    throw std::runtime_error("client: connection failed");
  }
}

void ClientConnection::sendString(const std::string &msg) {
  auto bytesReceived{send(s_.fd(), msg.data(), msg.size(), 0)};
  if (bytesReceived > 0) {
    std::cout << "client: message sent" << '\n';
  } else if (bytesReceived == 0) {
    std::cout << "client: closed connection" << '\n';
  } else {
    throw std::runtime_error("client: connection failed");
  }
}
