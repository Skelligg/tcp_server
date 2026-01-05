#pragma once

#include <cstdint>
struct MessageHeader {
  uint16_t type;
  uint16_t length;
};

enum class MessageType : uint16_t { CHAT = 1, PING = 2, ERROR = 3 };
