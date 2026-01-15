#pragma once

#include "protocol_handler.hpp"
#include "protocols.hpp"
#include "socket.hpp"
#include <string>
#include <vector>

class ClientConnection {
public:
  explicit ClientConnection(Socket s);

  SendResult sendMsg();
  void queueMsg(const MessageType type, const std::string &msg);
  RecvResult recvMsg();
  int fd() const { return s_.fd(); }

private:
  Socket s_;
  ProtocolHandler protocolHandler_;
};
