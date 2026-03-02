#include "MessageAck.h"
#include <SensorUnitManager/SensorUnitManager.h>
#define MAXTIMEOUT 10000

MessageAck::MessageAck() {}

uint8_t MessageAck::getSuArrInd(unsigned long long id) {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
    return 255;
  }
  bool found = false;
  int i;
  for (i = 0; i < suCount; i++) {
    if (id == idArray[i]) {
      found = 1;
      break;
    }
  }

  uint8_t temp = found ? i : 255;
  xSemaphoreGiveRecursive(mutex);
  return temp;
}

void MessageAck::addSensorUnit(unsigned long long suID) {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
    return;
  } else if (suCount < MAXPEERS) {
    idArray[suCount++] = suID;
  } else {
    Serial.println("Max peers reached");
  }

  xSemaphoreGiveRecursive(mutex);
}

void MessageAck::expectPacket(unsigned long long suID) {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
    return;
  }
  uint8_t ind;

  if ((ind = getSuArrInd(suID)) != 255) {
    packetsRequested[ind]++;
  } else {
    Serial.println("Invalid su array index");
  }
  xSemaphoreGiveRecursive(mutex);
}

void MessageAck::packetReceived(unsigned long long suID) {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
    return;
  }

  uint8_t ind = getSuArrInd(suID);
  if (ind != 255) {
    packetsReceived[ind]++;
  } else {
    Serial.println("Invalid su array index");
  }

  xSemaphoreGiveRecursive(mutex);
}

double MessageAck::getPacketDropPercentage(unsigned long long suID) {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
    return -1.0;
  }

  uint8_t ind;
  double temp;
  if ((ind = getSuArrInd(suID)) != 255) {
    temp = static_cast<double>(packetsReceived[ind]) /
           static_cast<double>(packetsRequested[ind]);
  } else {
    Serial.println("Invalid su array index");
    temp = -1;
  }
  xSemaphoreGiveRecursive(mutex);
  return temp;
}

/**
@brief: destructor frees internal ackListItem array
*/
MessageAck::~MessageAck() {}
