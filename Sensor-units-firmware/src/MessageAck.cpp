#include "MessageAck.h"
#include <optional>

MessageAck::MessageAck() {
  ackArr = new ackListItem[BEGINACKLEN];
  arrSize = sizeof(arrSize);
}

void MessageAck::resize() {
  if (ackArr == nullptr) {
    Serial.println("Resize failed, object was never initialized");
    return;
  }

  ackListItem *newArr = new ackListItem[capacity * 2];
  memcpy(newArr, ackArr, sizeof(ackArr));
  capacity *= 2;
  ackArr = newArr;
}

std::optional<ackListItem> MessageAck::insert(unsigned long long msgID,
                                              const Packet &p) {}

void MessageAck::remove(unsigned long long msgID) {}
