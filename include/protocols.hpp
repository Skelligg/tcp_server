#pragma once

#include <cstddef>
#include <cstdint>
struct MessageHeader {
  uint16_t type;
  uint16_t length;
};

constexpr size_t MessageHeaderSize{sizeof(MessageHeader)};

enum class MessageType : uint16_t { CHAT = 1, PING = 2, ERROR = 3 };
