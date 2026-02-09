#include "SensorUnitManager.h"

void SensorUnitReadings::postReading(const Packet &p) {
  if (xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) {
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
  if (!found) {
    Serial.println("Failed to post packet, update firmware for new sensors "
                   "that were added");
  }
  xSemaphoreGiveRecursive(mutex);
}

int SensorUnitReadings::getReadingCount() {
  if (xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
  }

  int returnVal = count;

  xSemaphoreGiveRecursive(mutex);
  return returnVal;
}

Packet &SensorUnitReadings::getReading(PacketInfo_t packet) {
  if (xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) {
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
