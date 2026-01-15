#include "protocol_handler.hpp"
#include "protocols.hpp"
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

ProtocolHandler::ProtocolHandler()
    : readBuffer_{}, sendBuffer_{}, sendOffset_{0}, readOffset_{0},
      readState_(ReadState::Header) {}

void ProtocolHandler::pushIncoming(std::span<const std::byte> buffer) {
  readBuffer_.insert(readBuffer_.end(), buffer.begin(), buffer.end());

  std::cout << "client : received message" << '\n';
}

void ProtocolHandler::tryReadMessage() {
  MessageHeader header;
  bool messageRead{false};
  while (true) {
    if (readState_ == ReadState::Header &&
        (readBuffer_.size() - readOffset_) >= MessageHeaderSize) {
      std::memcpy(&header, readBuffer_.data() + readOffset_,
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
        (readBuffer_.size() - readOffset_) >= header.length) {

      const auto *data =
          reinterpret_cast<const char *>(readBuffer_.data() + readOffset_);

      std::string msg{data, data + header.length};
      std::cout << "body: \"" << msg << "\"" << '\n';
      readOffset_ += header.length;
      readState_ = ReadState::Header;
      messageRead = true;
    } else
      break;
  };

  if (readOffset_ > 0) {
    readBuffer_.erase(readBuffer_.begin(), readBuffer_.begin() + readOffset_);
    readOffset_ = 0;
  }
}

SendResult ProtocolHandler::sendBytes() {}

void ProtocolHandler::queueOutgoing(const MessageType type,
                                    const std::string &data) {
  const std::size_t newBytes{MessageHeaderSize + data.size()};
  const std::size_t bufferSize{sendBuffer_.size()};

  sendBuffer_.resize(bufferSize + newBytes);

  MessageHeader header{htons(static_cast<uint16_t>(type)), htons(data.size())};

  std::memcpy(sendBuffer_.data() + bufferSize, &header, MessageHeaderSize);
  std::memcpy(sendBuffer_.data() + bufferSize + sizeof(header), data.data(),
              data.size());
}
