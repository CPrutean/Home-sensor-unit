#include "MessageAck.h"

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

void MessageAck::insert(unsigned long long msgID, int count) {
  if (ackArr == nullptr) {
    Serial.println("Insert failed object was never initialized");
    return;
  }
  if (size >= capacity) {
    resize();
  }
  ackArr[size] = {msgID, count};
  size++;
}

void MessageAck::remove(unsigned long long msgID) {
  if (ackArr == nullptr) {
    Serial.println("remove failed object was never initialized");
    return;
  }
  bool found = false;
  int i;
  for (i = 0; i < size; i++) {
    if (ackArr[i].msgID == msgID) {
      found = true;
      break;
    }
  }

  if (!found) {
    Serial.println("Object was never found");
    return;
  }

  if (ackArr[i].count > 0) {
    ackArr[i].count--;
    return;
  } else {
    for (int j = i; j < size - 1; j++) {
      ackArr[j] = ackArr[j + 1];
    }
    size--;
    arrSize -= sizeof(ackListItem);
  }
}
