#include "SensorUnitManager.h"

void SensorUnitReadings::postReading(const Packet &p) {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
  }

  bool found = false;
  for (uint8_t i{0}; i < count; i++) {
    if (packets[i].info == p.info) {
      packets[i] = p;
      found = true;
      break;
    }
  }

  if (!found && count >= size) {
    Serial.println("No space for new packet, update firmware");
  } else if (!found) {
    packets[count] = p;
    count++;
  } else {
    Serial.println("Packet was found and placed properly");
  }
  xSemaphoreGiveRecursive(mutex);
}

int SensorUnitReadings::getReadingCount() {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
  }

  int returnVal = count;

  xSemaphoreGiveRecursive(mutex);
  return returnVal;
}

Packet &SensorUnitReadings::getReading(PacketInfo_t packet) {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
  }

  Packet *result = &packets[0];
  for (uint8_t i{0}; i < count; i++) {
    if (packets[i].info == packet) {
      result = &packets[i];
      break;
    }
  }

  xSemaphoreGiveRecursive(mutex);
  return *result;
}
