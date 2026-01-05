#pragma once

#include "protocols.hpp"
#include "socket.hpp"
#include <string>
#include <vector>

class ClientConnection {
public:
  explicit ClientConnection(Socket s);

  void sendMsg(const MessageType type, const std::string &data);
  void recvSome();
  int fd() const { return s_.fd(); }

private:
  Socket s_;
  std::vector<std::byte> receiveBuffer_;
};
