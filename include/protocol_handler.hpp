#pragma once

#include "protocols.hpp"
#include <span>
#include <string>
#include <vector>

enum class ReadState { Header, Body };

class ProtocolHandler {
public:
  ProtocolHandler();
  void pushIncoming(std::span<const std::byte> buffer);
  void tryReadMessage();
  SendResult sendBytes();
  void queueOutgoing(const MessageType type, const std::string &data);

private:
  std::vector<std::byte> readBuffer_;
  std::vector<std::byte> sendBuffer_;
  size_t sendOffset_;
  size_t readOffset_;
  ReadState readState_;
};
