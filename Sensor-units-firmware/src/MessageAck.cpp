#include "MessageAck.h"
#define MAXTIMEOUT 10000

/*
@breif: Default constructor for MessageAck object
*/
MessageAck::MessageAck() {
  ackArr = new ackListItem[BEGINACKLEN];
  arrSize = BEGINACKLEN;
  mutex = xSemaphoreCreateRecursiveMutex();
}

/*
@breif: Resizes internal array for improved performance when needed
*/
void MessageAck::resize() {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take MUTEX");
    return;
  }


  if (ackArr == nullptr) {
    Serial.println("Resize failed, object was never initialized");
    xSemaphoreGiveRecursive(mutex);
    return;
  }

  size_t newCapacity = capacity * 2;
  ackListItem *newArr = new ackListItem[newCapacity];
  memcpy(newArr, ackArr, sizeof(ackListItem) * size);
  delete[] ackArr;
  capacity = newCapacity;
  ackArr = newArr;
  xSemaphoreGiveRecursive(mutex);
}


/*
@breif: inserts a recieved packet and groups them with similar messages with the same request ID
@param: const Packet& p: Packet recieved to be grouped with internal msgID 
@return: will return an ackListItem when the final packet is recieved with the Packet::FIN type
*/
etl::optional<ackListItem> MessageAck::insertPacket(const Packet &p) {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
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
    ackListItem ret = ackArr[i];
    xSemaphoreGiveRecursive(mutex);
    return ret;
  }
}

/*
@breif: removes an ackArrItem based on the group ID passed
@param unsigned long long msgID: message group ID to be removed
*/
void MessageAck::removeAckArrItem(unsigned long long msgID) {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
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

  for (int j{i}; j < static_cast<int>(size)-1; j++) {
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
  size++;

  xSemaphoreGiveRecursive(mutex);
}

// Remove timed out requests and return list of SU indexes that timed out
void MessageAck::removeTimedOutReq(int* suNumList) {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
    return;
  }

  const unsigned long now = millis();
  size_t writeIdx = 0;
  for (size_t readIdx = 0; readIdx < size; ++readIdx) {
    unsigned long elapsed = now - ackArr[readIdx].postTime;
    if (elapsed > MAXTIMEOUT) {
      // Optionally record timed out SU index if mapping exists; placeholder -1
      if (suNumList) {
        suNumList[readIdx] = -1;
      }
      continue; // drop this item
    }
    if (writeIdx != readIdx) {
      ackArr[writeIdx] = ackArr[readIdx];
    }
    ++writeIdx;
  }
  size = writeIdx;
  xSemaphoreGiveRecursive(mutex);
}

/*
@breif: destructor frees internal ackListItem array
*/
MessageAck::~MessageAck() {
  delete[] ackArr;
}