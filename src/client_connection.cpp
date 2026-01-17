#include "client_connection.hpp"
#include "protocol_handler.hpp"
#include "protocols.hpp"
#include "socket.hpp"
#include <asm-generic/errno.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <system_error>
#include <utility>
#include <vector>

ClientConnection::ClientConnection(Socket s)
    : s_{std::move(s)}, protocolHandler_{} {}

RecvResult ClientConnection::recvMsg() {
  std::vector<std::byte> buffer{1024};

  ssize_t n{recv(s_.fd(), buffer.data(), buffer.size(), 0)};

  if (n == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return RecvResult::WouldBlock;
    } else
      return RecvResult::Error;
  }

  if (n == 0) {
    // Client disconnected
    std::cout << "client disconnected\n";
    return RecvResult::Disconnected;
  }

  protocolHandler_.pushIncoming(
      std::span{buffer.data(), static_cast<size_t>(n)});

  auto packetOpt{protocolHandler_.tryReadPacket()};

  if (packetOpt) {
    const Packet &packet{*packetOpt};
    std::string msg(reinterpret_cast<const char *>(packet.body.data()),
                    packet.header.length);
    std::cout << "Received packet body: " << msg << '\n';
    return RecvResult::MessageRead;
  } else
    return RecvResult::PartialMessageRead;
}

SendResult ClientConnection::sendMsg() {
  std::span<const std::byte> sendBuffer_{protocolHandler_.getOutgoingBytes()};
  ssize_t n{send(s_.fd(), sendBuffer_.data(), sendBuffer_.size(), 0)};
  if (n == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return SendResult::WouldBlock;
    } else {
      return SendResult::Error;
    }
  }

  if (n == 0) {
    return SendResult::Disconnected;
  }

  return protocolHandler_.adjustSendOffset(n);
}

void ClientConnection::queueMsg(const MessageType type,
                                const std::string &msg) {
  protocolHandler_.queueOutgoing(type, msg);
}
