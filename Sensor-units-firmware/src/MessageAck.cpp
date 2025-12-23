#include "MessageAck.h"
#define MAXTIMEOUT 10000

MessageAck::MessageAck() {
  ackArr = new ackListItem[BEGINACKLEN];
  arrSize = sizeof(arrSize);
  mutex = xSemaphoreCreateRecursiveMutex();
}

void MessageAck::resize() {
  if (!xSemaphoreTakeRecursive(mutex, portMAX_DELAY) == pdTRUE) {
    Serial.println("Failed to take MUTEX");
    return;
  }


  if (ackArr == nullptr) {
    Serial.println("Resize failed, object was never initialized");
    xSemaphoreGiveRecursive(mutex);
    return;
  }

  ackListItem *newArr = new ackListItem[capacity * 2];
  memcpy(newArr, ackArr, sizeof(ackArr));
  capacity *= 2;
  ackArr = newArr;
}


etl::optional<ackListItem> MessageAck::insertPacket(unsigned long long msgID, const Packet &p) {
  if (!xSemaphoreTakeRecursive(mutex, portMAX_DELAY) == pdTRUE) {
    Serial.println("Failed to take mutex in MessageAck");
    return etl::nullopt;
  }


  bool found = false;
  int i;
  for (i = 0; i < size; i++) {
    if (msgID == ackArr[i].msgID) {
      found = true;
      break;
    }
  }
  if (!found) {
    Serial.println("INVALID MSGID");
    xSemaphoreGiveRecursive(mutex);
    return etl::nullopt;
  }

  if (p.type != Packet::FIN) {
    ackArr[i].packetList[ackArr[i].packetCount++] = p;
    xSemaphoreGiveRecursive(mutex);
    return etl::nullopt;
  } else {
    xSemaphoreGiveRecursive(mutex);
    return ackArr[i];
  }
}

void MessageAck::removeAckArrItem(unsigned long long msgID) {
  if (!xSemaphoreTakeRecursive(mutex, portMAX_DELAY) == pdTRUE) {
    Serial.println("Failed to take mutex");
    return;
  }


  int i;
  bool found = false;
  for (i = 0; i < size; i++) {
    if (msgID == ackArr[i].msgID) {
      found = true;
      break;
    }
  }

  if (!found) {
    Serial.println("Invalid msgID");
    xSemaphoreGiveRecursive(mutex);
    return;
  }

  for (int j{i}; j < capacity-1; j++) {
    ackArr[j] = ackArr[j+1];
  }
  size--;
  xSemaphoreGiveRecursive(mutex);
}

void MessageAck::addNewAckArrItem(unsigned long long msgID) {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
    return;
  }


  if (size >= capacity) {
    resize();
  }
  ackArr[size] = ackListItem{msgID, millis()};

  xSemaphoreGiveRecursive(mutex);
}


MessageAck::~MessageAck() {
  delete[] ackArr;
}