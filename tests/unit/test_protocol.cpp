#include <gtest/gtest.h>
#include <string>

#include "client_connection.hpp"
#include "protocol_handler.hpp"
#include "protocols.hpp"

TEST(ProtocolTest, HandlesSerialisation) {
  // setup
  ProtocolHandler handler;
  std::string msg{"PING"};
  handler.queueOutgoing(MessageType::PING, msg);
  std::span sendBuffer{handler.getOutgoingBytes()};

  // assert
  EXPECT_EQ(sendBuffer.size(), ProtocolHeaderSize + 4);
}
