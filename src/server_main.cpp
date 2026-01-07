#include "client_connection.hpp"
#include "protocols.hpp"
#include "tcp_server.hpp"
#include <iostream>

int main() {
  TcpServer server{"8080", 10};

  server.start();
  std::cout << "server: running on port 8080" << '\n';
  while (true) {

    ClientConnection connectedClient{server.acceptConnection()};
    std::string toSend{"OK"};
    connectedClient.sendMsg(MessageType::PING, toSend);
    break;
  }

  return 0;
}
