#pragma once

#include "protocols.hpp"
#include "socket.hpp"
#include <string>
#include <vector>

enum class ReadState { Header, Body };

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

class ClientConnection {
public:
  explicit ClientConnection(Socket s);

  SendResult sendMsg();
  void queueMsg(const MessageType type, const std::string &msg);
  RecvResult recvMsg();
  int fd() const { return s_.fd(); }

private:
  Socket s_;
  std::vector<std::byte> receiveBuffer_;
  std::vector<std::byte> sendBuffer_;
  size_t sendOffset_;
  size_t readOffset_;
  ReadState readState_;
};
