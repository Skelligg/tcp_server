#pragma once

#include "socket.hpp"
#include <string>

class ClientConnection {
public:
  explicit ClientConnection(Socket s);

  void sendString(const std::string &);
  void recvSome();
  int fd() const { return s_.fd(); }

private:
  Socket s_;
};
