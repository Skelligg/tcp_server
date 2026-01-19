
#include "client_connection.hpp"
#include "protocols.hpp"
#include "tcp_client.hpp"
#include "tcp_server.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

static void run_server() {
  try {
    TcpServer server{"8080", 10};
    server.start();
    server.run(); // blocking
  } catch (const std::exception &e) {
    std::cerr << "Server error: " << e.what() << '\n';
    std::exit(1);
  }
}

int main() {
  std::cout << "[integration] starting server...\n";

  std::thread serverThread(run_server);

  // Give server time to start
  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  try {
    TcpClient client{"127.0.0.1", "8080"};
    ClientConnection conn{client.connect()};

    std::cout << "[integration] client connected\n";

    conn.queueMsg(MessageType::PING, "PING");
    conn.sendMsg();

    // Poll for response
    for (;;) {
      auto result = conn.recvMsg();

      if (result == RecvResult::MessageRead) {
        std::cout << "[integration] received response\n";
        break;
      }

      if (result == RecvResult::Disconnected || result == RecvResult::Error) {
        std::cerr << "[integration] server disconnected\n";
        std::exit(1);
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "[integration] ping–pong SUCCESS\n";
  } catch (const std::exception &e) {
    std::cerr << "[integration] test failed: " << e.what() << '\n';
    std::exit(1);
  }

  std::cout << "[integration] done\n";

  std::exit(0);
}
