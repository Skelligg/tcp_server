#pragma once

#include <cstddef>
#include <cstdint>
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
