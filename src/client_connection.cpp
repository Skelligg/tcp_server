#include "client_connection.hpp"
#include "protocol_handler.hpp"
#include "protocols.hpp"
#include "socket.hpp"
#include <asm-generic/errno.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <system_error>
#include <utility>
#include <vector>

ClientConnection::ClientConnection(Socket s)
    : s_{std::move(s)}, protocolHandler_{} {}

RecvResult ClientConnection::recvMsg() {
  std::vector<std::byte> buffer{1024};
  MessageHeader header;
  bool messageRead{false};

  ssize_t n{recv(s_.fd(), buffer.data(), buffer.size(), 0)};

  if (n == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return RecvResult::WouldBlock;
    } else
      return RecvResult::Error;
  }

  if (n == 0) {
    // Client disconnected
    std::cout << "client disconnected\n";
    return RecvResult::Disconnected;
  }

  protocolHandler_.pushIncoming(
      std::span{buffer.data(), static_cast<size_t>(n)});
  return RecvResult::MessageRead;
}

SendResult ClientConnection::sendMsg() {
  ssize_t n{send(s_.fd(), sendBuffer_.data() + sendOffset_,
                 sendBuffer_.size() - sendOffset_, 0)};
  if (n == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return SendResult::WouldBlock;
    } else {
      return SendResult::Error;
    }
  }

  if (n == 0) {
    return SendResult::Disconnected;
  }
  sendOffset_ += n;

  if (sendOffset_ == sendBuffer_.size()) {
    sendBuffer_.clear();
    sendOffset_ = 0;
    return SendResult::MessageSent;
  }
  return SendResult::PartialSend;
}

void ClientConnection::queueMsg(const MessageType type,
                                const std::string &msg) {
  protocolHandler_.queueOutgoing(type, msg);
}
