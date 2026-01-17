#pragma once

#include "protocols.hpp"
#include <optional>
#include <span>
#include <string>
#include <vector>

enum class ReadState { Header, Body };

class ProtocolHandler {
public:
  ProtocolHandler();
  void pushIncoming(std::span<const std::byte> buffer);
  std::optional<Packet> tryReadPacket();
  std::span<const std::byte> getOutgoingBytes();
  SendResult adjustSendOffset(ssize_t bytesSent);
  void queueOutgoing(const MessageType type, const std::string &data);

private:
  std::vector<std::byte> readBuffer_;
  std::vector<std::byte> sendBuffer_;
  size_t sendOffset_;
  size_t readOffset_;
  ReadState readState_;
  Packet currentPacket_;
};
