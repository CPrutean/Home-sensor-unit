#pragma once
#include "global_include.h"

#define BEGINACKLEN 8
class MessageAck final {
public:
  MessageAck();
  etl::optional<ackListItem> insertPacket(unsigned long long msgID, const Packet &p);
  void addNewAckArrItem(unsigned long long msgID);
  void removeAckArrItem(unsigned long long msgID);
  void removeTimedOutReq(int* suNumList);
  ~MessageAck();
private:
  ackListItem *ackArr{nullptr};
  void resize();
  size_t capacity{BEGINACKLEN};
  size_t size{0};
  size_t arrSize{};
  SemaphoreHandle_t mutex{};
};
