#include "protocol_handler.hpp"
#include "protocols.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

ProtocolHandler::ProtocolHandler()
    : readBuffer_{}, sendBuffer_{}, sendOffset_{0}, readOffset_{0},
      readState_(ReadState::Header), currentHeader_{} {}

void ProtocolHandler::pushIncoming(std::span<const std::byte> buffer) {
  readBuffer_.insert(readBuffer_.end(), buffer.begin(), buffer.end());
}

RecvResult ProtocolHandler::tryReadMessage() {
  bool messageRead{false};
  while (true) {
    if (readState_ == ReadState::Header &&
        (readBuffer_.size() - readOffset_) >= MessageHeaderSize) {
      std::memcpy(&currentHeader_, readBuffer_.data() + readOffset_,
                  sizeof(MessageHeader));
      currentHeader_.length = ntohs(currentHeader_.length);
      currentHeader_.type = ntohs(currentHeader_.type);

      if (currentHeader_.length == 0)
        throw std::runtime_error("server: msg length cannot be empty");
      if (currentHeader_.length > 1024)
        throw std::runtime_error{"server: msg length is too big"};

      readOffset_ += MessageHeaderSize;

      std::cout << "type: " << currentHeader_.type << '\n';
      std::cout << "length: " << currentHeader_.length << '\n';

      readState_ = ReadState::Body;
    } else
      break;

    if (readState_ == ReadState::Body &&
        (readBuffer_.size() - readOffset_) >= currentHeader_.length) {

      const auto *data =
          reinterpret_cast<const char *>(readBuffer_.data() + readOffset_);

      std::string msg{data, data + currentHeader_.length};
      std::cout << "body: \"" << msg << "\"" << '\n';
      readOffset_ += currentHeader_.length;
      readState_ = ReadState::Header;
      messageRead = true;
    } else
      break;
  };

  if (messageRead) {
    return RecvResult::MessageRead;
  }

  if (readOffset_ > 0) {
    readBuffer_.erase(readBuffer_.begin(), readBuffer_.begin() + readOffset_);
    readOffset_ = 0;
  }

  return RecvResult::PartialMessageRead;
}

std::span<const std::byte> ProtocolHandler::getOutgoingBytes() {
  return std::span{sendBuffer_.data() + sendOffset_,
                   sendBuffer_.size() - sendOffset_};
}

SendResult ProtocolHandler::adjustSendOffset(ssize_t bytesSent) {
  sendOffset_ += bytesSent;
  if (sendOffset_ == sendBuffer_.size()) {
    sendBuffer_.clear();
    sendOffset_ = 0;
    return SendResult::MessageSent;
  } else
    return SendResult::PartialSend;
}

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
