#include "client_connection.hpp"
#include "protocols.hpp"
#include "socket.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <utility>
#include <vector>

ClientConnection::ClientConnection(Socket s)
    : s_{std::move(s)}, receiveBuffer_{} {}

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

void ClientConnection::sendMsg(const MessageType type, const std::string &msg) {
  constexpr std::size_t headerSize{sizeof(MessageHeader)};
  const std::size_t totalSize{headerSize + msg.size()};

  std::size_t bytesLeft{totalSize};

  MessageHeader header{htons(static_cast<uint16_t>(type)), htons(msg.size())};

  std::vector<char> buffer(totalSize);
  std::memcpy(buffer.data(), &header, headerSize);
  std::memcpy(buffer.data() + sizeof(header), msg.data(), msg.size());
  size_t bytesSent{0};

  while (bytesSent < totalSize) {
    ssize_t n = send(s_.fd(), buffer.data() + bytesSent, bytesLeft, 0);
    if (n == -1)
      throw std::system_error(errno, std::system_category(), "send failed");

    bytesSent += n;
    bytesLeft -= n;
  }
  std::cout << "server: sent " << bytesSent << " bytes" << '\n';
}
