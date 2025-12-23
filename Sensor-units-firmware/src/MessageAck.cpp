#include "MessageAck.h"
#define MAXTIMEOUT 10000

/*
@breif: Default constructor for MessageAck object
*/
MessageAck::MessageAck() {
  ackArr = new ackListItem[BEGINACKLEN];
  arrSize = sizeof(arrSize);
  mutex = xSemaphoreCreateRecursiveMutex();
}

/*
@breif: Resizes internal array for improved performance when needed
*/
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


/*
@breif: inserts a recieved packet and groups them with similar messages with the same request ID
@param: const Packet& p: Packet recieved to be grouped with internal msgID 
@return: will return an ackListItem when the final packet is recieved with the Packet::FIN type
*/
etl::optional<ackListItem> MessageAck::insertPacket(const Packet &p) {
  if (!xSemaphoreTakeRecursive(mutex, portMAX_DELAY) == pdTRUE) {
    Serial.println("Failed to take mutex in MessageAck");
    return etl::nullopt;
  }


  bool found = false;
  int i;
  for (i = 0; i < size; i++) {
    if (p.msgID == ackArr[i].msgID) {
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

/*
@breif: removes an ackArrItem based on the group ID passed
@param unsigned long long msgID: message group ID to be removed
*/
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
/*
@breif: creates a new ackListItem object with a new msg groupID
@param unsigned long long msgID: message group ID to associate with different packets
*/
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

/*
@breif: destructor frees internal ackListItem array
*/
MessageAck::~MessageAck() {
  delete[] ackArr;
}