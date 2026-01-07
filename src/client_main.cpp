#include "client_connection.hpp"
#include "tcp_client.hpp"
#include <iostream>

int main() {

  TcpClient client{"127.0.0.1", "8080"};

  ClientConnection connectedClient{client.connect()};
  std::cout << "client: connected to server on port 8080" << '\n';

  connectedClient.recvMsg();

  return 0;
}
