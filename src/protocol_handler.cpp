#include "protocol_handler.hpp"
#include "protocols.hpp"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <optional>
#include <stdexcept>
#include <system_error>
#include <utility>
#include <vector>

ProtocolHandler::ProtocolHandler()
    : readBuffer_{}, sendBuffer_{}, sendOffset_{0}, readOffset_{0},
      readState_(ReadState::Header), currentPacket_{} {}

void ProtocolHandler::pushIncoming(std::span<const std::byte> buffer) {
  readBuffer_.insert(readBuffer_.end(), buffer.begin(), buffer.end());
}

std::optional<Packet> ProtocolHandler::tryReadPacket() {
  while (true) {
    if (readState_ == ReadState::Header &&
        (readBuffer_.size() - readOffset_) >= ProtocolHeaderSize) {
      std::memcpy(&currentPacket_.header, readBuffer_.data() + readOffset_,
                  sizeof(ProtocolHeader));
      currentPacket_.header.length = ntohs(currentPacket_.header.length);
      currentPacket_.header.type = ntohs(currentPacket_.header.type);

      if (currentPacket_.header.length == 0)
        throw std::runtime_error("server: msg length cannot be empty");
      if (currentPacket_.header.length > 1024)
        throw std::runtime_error{"server: msg length is too big"};

      readOffset_ += ProtocolHeaderSize;

      readState_ = ReadState::Body;
    } else
      break;

    if (readState_ == ReadState::Body &&
        (readBuffer_.size() - readOffset_) >= currentPacket_.header.length) {

      currentPacket_.body.resize(currentPacket_.header.length);
      std::memcpy(currentPacket_.body.data(), readBuffer_.data() + readOffset_,
                  currentPacket_.header.length);

      readOffset_ += currentPacket_.header.length;
      readState_ = ReadState::Header;
      return std::optional{currentPacket_};
    } else
      break;
  };

  if (readOffset_ > 0) {
    readBuffer_.erase(readBuffer_.begin(), readBuffer_.begin() + readOffset_);
    readOffset_ = 0;
  }

  return std::nullopt;
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
  const std::size_t newBytes{ProtocolHeaderSize + data.size()};
  const std::size_t bufferSize{sendBuffer_.size()};

  sendBuffer_.resize(bufferSize + newBytes);

  ProtocolHeader header{htons(static_cast<uint16_t>(type)), htons(data.size())};

  std::memcpy(sendBuffer_.data() + bufferSize, &header, ProtocolHeaderSize);
  std::memcpy(sendBuffer_.data() + bufferSize + sizeof(header), data.data(),
              data.size());
}
