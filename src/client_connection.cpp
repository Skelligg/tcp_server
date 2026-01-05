#include "client_connection.hpp"
#include "protocols.hpp"
#include "socket.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <system_error>
#include <utility>
#include <vector>

ClientConnection::ClientConnection(Socket s)
    : s_{std::move(s)}, receiveBuffer_{} {}

void ClientConnection::recvMsg() {
  std::vector<char> buffer(1024);
  MessageHeader header;
  bool READING_HEADER{true};
  bool READING_BODY{false};
  size_t readOffset{0};

  while (true) {
    ssize_t n{recv(s_.fd(), buffer.data(), buffer.size(), 0)};

    if (n == -1)
      throw std::system_error(errno, std::system_category(), "recv failed");

    receiveBuffer_.insert(receiveBuffer_.end(),
                          reinterpret_cast<std::byte *>(buffer.data()),
                          reinterpret_cast<std::byte *>(buffer.data()) + n);

    while (true) {
      if (READING_HEADER &&
          (receiveBuffer_.size() - readOffset) >= MessageHeaderSize) {
        std::memcpy(&header, receiveBuffer_.data() + readOffset,
                    sizeof(MessageHeader));
        header.length = ntohs(header.length);
        header.type = ntohs(header.type);

        if (header.length == 0)
          throw std::runtime_error("server: msg length cannot be empty");
        if (header.length > 1024)
          throw std::runtime_error{"server: msg length is too big"};

        readOffset += MessageHeaderSize;

        READING_HEADER = false;
        READING_BODY = true;
      } else
        break;

      if (READING_BODY &&
          (receiveBuffer_.size() - readOffset) >= header.length) {

        const auto *data =
            reinterpret_cast<const char *>(receiveBuffer_.data() + readOffset);

        std::string msg{data, data + header.length};
        std::cout << "server : msg received: " << msg << '\n';
        readOffset += header.length;
        READING_BODY = false;
        READING_HEADER = true;
      } else
        break;
    }

    if (readOffset > 0) {
      receiveBuffer_.erase(receiveBuffer_.begin(),
                           receiveBuffer_.begin() + readOffset);
      readOffset = 0;
    }
  };
}

void ClientConnection::sendMsg(const MessageType type, const std::string &msg) {
  const std::size_t totalSize{MessageHeaderSize + msg.size()};

  size_t bytesLeft{totalSize};

  MessageHeader header{htons(static_cast<uint16_t>(type)), htons(msg.size())};

  std::vector<char> buffer(totalSize);
  std::memcpy(buffer.data(), &header, MessageHeaderSize);
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
