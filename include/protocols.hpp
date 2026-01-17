#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

struct ProtocolHeader {
  uint16_t type;
  uint16_t length;
  // to add/change later
  // protocol name
  // length to buffer_size
  // major version & minor version
  // reserved - space for future protocol changes
};

struct Packet {
  ProtocolHeader header;
  std::vector<std::byte> body;
};

constexpr size_t ProtocolHeaderSize{sizeof(ProtocolHeader)};

enum class MessageType : uint16_t { CHAT = 1, PING = 2, ERROR = 3 };

[[nodiscard]] inline MessageType getMessageType(uint16_t type) {
  switch (type) {
  case static_cast<uint16_t>(MessageType::CHAT):
    return MessageType::CHAT;
  case static_cast<uint16_t>(MessageType::PING):
    return MessageType::PING;
  case static_cast<uint16_t>(MessageType::ERROR):
    return MessageType::ERROR;
  default:
    throw std::runtime_error("MessageType not found");
  }
}

enum class RecvResult {
  WouldBlock,
  MessageRead,
  Disconnected,
  Error,
  PartialMessageRead
};
enum class SendResult {
  WouldBlock,
  MessageSent,
  PartialSend,
  Disconnected,
  Error
};
