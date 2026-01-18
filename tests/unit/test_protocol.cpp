#include <arpa/inet.h>
#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <string>
#include <sys/types.h>

#include "client_connection.hpp"
#include "protocol_handler.hpp"
#include "protocols.hpp"

class ProtocolHandlerTest : public testing::Test {
protected:
  ProtocolHandlerTest() {}

  ProtocolHandler sendHandler;
  ProtocolHandler readHandler;
};

TEST_F(ProtocolHandlerTest, HandlesSerialisation) {
  // setup
  const size_t ProtocolHeaderFieldSize{2};
  std::string msg{"PING"};
  sendHandler.queueOutgoing(MessageType::PING, msg);
  std::span sendBuffer{sendHandler.getOutgoingBytes()};

  readHandler.pushIncoming(sendBuffer);

  std::optional<Packet> packetOpt;
  while (!packetOpt) {
    packetOpt = readHandler.tryReadPacket();
  }

  uint16_t messageType;
  uint16_t length;
  std::string msgBody;

  std::memcpy(&messageType, &packetOpt->header.type, ProtocolHeaderFieldSize);
  std::memcpy(&length, &packetOpt->header.length, ProtocolHeaderFieldSize);
  msgBody.assign(reinterpret_cast<const char *>(packetOpt->body.data()),
                 length);

  // assert
  EXPECT_EQ(sendBuffer.size(), ProtocolHeaderSize + 4);
  EXPECT_EQ(convertToMessageType(messageType), MessageType::PING);
  EXPECT_EQ(msgBody, "PING");
}

TEST_F(ProtocolHandlerTest, HandlesMultipleSerialisations) {
  // setup
  sendHandler.queueOutgoing(MessageType::PING, "A");
  sendHandler.queueOutgoing(MessageType::PING, "BC");

  std::span<const std::byte> sendBuffer{sendHandler.getOutgoingBytes()};

  size_t offset = 0;

  auto readHeader = [&](uint16_t &type, uint16_t &length) {
    std::memcpy(&type, sendBuffer.data() + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    std::memcpy(&length, sendBuffer.data() + offset, sizeof(uint16_t));
    offset += sizeof(uint16_t);

    type = ntohs(type);
    length = ntohs(length);
  };

  // ---- First packet ----
  uint16_t type1, length1;
  readHeader(type1, length1);

  std::string body1(reinterpret_cast<const char *>(sendBuffer.data() + offset),
                    length1);
  offset += length1;

  // ---- Second packet ----
  uint16_t type2, length2;
  readHeader(type2, length2);

  std::string body2(reinterpret_cast<const char *>(sendBuffer.data() + offset),
                    length2);
  offset += length2;

  // assert
  EXPECT_EQ(convertToMessageType(type1), MessageType::PING);
  EXPECT_EQ(length1, 1);
  EXPECT_EQ(body1, "A");

  EXPECT_EQ(convertToMessageType(type2), MessageType::PING);
  EXPECT_EQ(length2, 2);
  EXPECT_EQ(body2, "BC");

  EXPECT_EQ(offset, sendBuffer.size());
}

TEST_F(ProtocolHandlerTest, PartialSendProgressionAcrossHandlers) {
  // Arrange
  std::string msg{"PING"};
  sendHandler.queueOutgoing(MessageType::PING, msg);

  const std::size_t headerSize = ProtocolHeaderSize;
  const std::size_t bodySize = msg.size();
  const std::size_t totalSize = headerSize + bodySize;

  // ---- Act 1: send header only ----
  auto outgoing1 = sendHandler.getOutgoingBytes();
  ASSERT_GE(outgoing1.size(), headerSize);

  readHandler.pushIncoming(outgoing1.first(headerSize));
  SendResult result1 = sendHandler.adjustSendOffset(headerSize);

  EXPECT_EQ(result1, SendResult::PartialSend);
  EXPECT_GT(sendHandler.getOutgoingBytes().size(), 0);

  // ---- Act 2: send partial body ----
  constexpr std::size_t partialBody = 2;
  auto outgoing2 = sendHandler.getOutgoingBytes();
  ASSERT_GE(outgoing2.size(), partialBody);

  readHandler.pushIncoming(outgoing2.first(partialBody));
  SendResult result2 = sendHandler.adjustSendOffset(partialBody);

  EXPECT_EQ(result2, SendResult::PartialSend);
  EXPECT_GT(sendHandler.getOutgoingBytes().size(), 0);

  // ---- Act 3: send remaining bytes ----
  auto outgoing3 = sendHandler.getOutgoingBytes();
  ASSERT_EQ(outgoing3.size(), totalSize - headerSize - partialBody);

  readHandler.pushIncoming(outgoing3);
  SendResult result3 = sendHandler.adjustSendOffset(outgoing3.size());

  EXPECT_EQ(result3, SendResult::MessageSent);
  EXPECT_EQ(sendHandler.getOutgoingBytes().size(), 0);

  // ---- Act 4: now read the fully received packet ----
  auto packetOpt = readHandler.tryReadPacket();
  ASSERT_TRUE(packetOpt.has_value());

  // ---- Assert packet correctness ----
  EXPECT_EQ(convertToMessageType(packetOpt->header.type), MessageType::PING);
  EXPECT_EQ(packetOpt->header.length, msg.size());

  EXPECT_EQ(std::string(reinterpret_cast<const char *>(packetOpt->body.data()),
                        packetOpt->body.size()),
            msg);
}
