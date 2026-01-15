#include "client_connection.hpp"
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
    : s_{std::move(s)}, receiveBuffer_{}, sendBuffer_{}, sendOffset_{0},
      readOffset_{0}, readState_(ReadState::Header) {}

RecvResult ClientConnection::recvMsg() {
  std::vector<char> buffer(1024);
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

  receiveBuffer_.insert(receiveBuffer_.end(),
                        reinterpret_cast<std::byte *>(buffer.data()),
                        reinterpret_cast<std::byte *>(buffer.data()) + n);

  std::cout << "client : received message" << '\n';
  while (true) {
    if (readState_ == ReadState::Header &&
        (receiveBuffer_.size() - readOffset_) >= MessageHeaderSize) {
      std::memcpy(&header, receiveBuffer_.data() + readOffset_,
                  sizeof(MessageHeader));
      header.length = ntohs(header.length);
      header.type = ntohs(header.type);

      if (header.length == 0)
        throw std::runtime_error("server: msg length cannot be empty");
      if (header.length > 1024)
        throw std::runtime_error{"server: msg length is too big"};

      readOffset_ += MessageHeaderSize;

      std::cout << "type: " << header.type << '\n';
      std::cout << "length: " << header.length << '\n';

      readState_ = ReadState::Body;
    } else
      break;

    if (readState_ == ReadState::Body &&
        (receiveBuffer_.size() - readOffset_) >= header.length) {

      const auto *data =
          reinterpret_cast<const char *>(receiveBuffer_.data() + readOffset_);

      std::string msg{data, data + header.length};
      std::cout << "body: \"" << msg << "\"" << '\n';
      readOffset_ += header.length;
      readState_ = ReadState::Header;
      messageRead = true;
    } else
      break;
  };

  if (readOffset_ > 0) {
    receiveBuffer_.erase(receiveBuffer_.begin(),
                         receiveBuffer_.begin() + readOffset_);
    readOffset_ = 0;
  }

  if (messageRead) {
    return RecvResult::MessageRead;
  }

  return RecvResult::PartialMessageRead;
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
  const std::size_t newBytes{MessageHeaderSize + msg.size()};
  const std::size_t bufferSize{sendBuffer_.size()};

  sendBuffer_.resize(bufferSize + newBytes);

  MessageHeader header{htons(static_cast<uint16_t>(type)), htons(msg.size())};

  std::memcpy(sendBuffer_.data() + bufferSize, &header, MessageHeaderSize);
  std::memcpy(sendBuffer_.data() + bufferSize + sizeof(header), msg.data(),
              msg.size());
}
