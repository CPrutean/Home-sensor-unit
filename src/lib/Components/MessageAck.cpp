#include "MessageAck.h"
#include <SensorUnitManager/SensorUnitManager.h>
#define MAXTIMEOUT 10000

/** 
@breif: Default constructor for MessageAck object
*/
MessageAck::MessageAck() {
  ackArr = new ackListItem[BEGINACKLEN];
  arrSize = BEGINACKLEN;
  mutex = xSemaphoreCreateRecursiveMutex();
}

/** 
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

/** 
@breif: removes an ackArrItem based on the group ID passed
@param: unsigned long long msgID: message group ID to be removed
*/
bool MessageAck::removeAckArrItem(unsigned long long msgID) {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
    return false;
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
    return false;
  }

  for (int j{i}; j < static_cast<int>(size)-1; j++) {
    ackArr[j] = ackArr[j+1];
  }
  size--;
  xSemaphoreGiveRecursive(mutex);
  return true;
}
/** 
@breif: creates a new ackListItem object with a new msg groupID
@param: message group ID to associate with different packets
*/
void MessageAck::addNewAckArrItem(unsigned long long msgID, unsigned long long postTime) {
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
/**
 * @breif: will remove timed out requests from the message ack queue and increment the error status of a request
 * @param: suNumList, an array of Sensor unit status objects. The index of the array corresponds to the sensor units inside the Sensor unit managers
 */
void MessageAck::removeTimedOutReq(SensorUnitStatus* suNumList) {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
    return;
  }
  const unsigned long now = millis();
  for (int i{0}; i < capacity; i++) {
    unsigned long time = now-ackArr[i].postTime;
    if (time > MAXTIMEOUT && suNumList[ackArr[i].suInd] != SensorUnitStatus::OFFLINE) {
      suNumList[ackArr[i].suInd] = static_cast<SensorUnitStatus>(static_cast<uint8_t>(suNumList[ackArr[i].suInd])+1);
    } //Assume that the device is just dropping packets at first, if more than 1 packet is faulty assume offline state
  }
  xSemaphoreGiveRecursive(mutex);
}

/** 
@breif: destructor frees internal ackListItem array
*/
MessageAck::~MessageAck() {
  delete[] ackArr;
}