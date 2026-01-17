#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <string>
#include <sys/types.h>

#include <arpa/inet.h>

#include "client_connection.hpp"
#include "protocol_handler.hpp"
#include "protocols.hpp"

class ProtocolHandlerTest : public testing::Test {
protected:
  ProtocolHandlerTest() {}

  ProtocolHandler handler;
};

TEST_F(ProtocolHandlerTest, HandlesSerialisation) {
  // setup
  const size_t ProtocolHeaderFieldSize{2};
  std::string msg{"PING"};
  handler.queueOutgoing(MessageType::PING, msg);
  std::span sendBuffer{handler.getOutgoingBytes()};

  uint16_t messageType;
  uint16_t length;
  std::string msgBody;

  std::memcpy(&messageType, sendBuffer.data(), ProtocolHeaderFieldSize);
  messageType = ntohs(messageType);
  std::memcpy(&length, sendBuffer.data() + ProtocolHeaderFieldSize,
              ProtocolHeaderFieldSize);
  msgBody.assign(reinterpret_cast<const char *>(sendBuffer.data() + length),
                 length);
  length = ntohs(length);

  handler.adjustSendOffset(sendBuffer.size());

  // assert
  EXPECT_EQ(sendBuffer.size(), ProtocolHeaderSize + 4);
  EXPECT_EQ(getMessageType(messageType), MessageType::PING);
  EXPECT_EQ(msgBody, "PING");
}

TEST_F(ProtocolHandlerTest, MessageQueueing) {
  std::string msg1{"A"};
  std::string msg2{"BC"};
  handler.queueOutgoing(MessageType::PING, msg1);
  handler.queueOutgoing(MessageType::PING, msg2);
  std::span sendBuffer{handler.getOutgoingBytes()};

  // assert
  EXPECT_EQ(sendBuffer.size(), 2 * ProtocolHeaderSize + 3);
}
