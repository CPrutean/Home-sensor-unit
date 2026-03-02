#include "SensorUnitManager.h"

void SensorUnitReadings::postReading(const Packet &p) {
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
    return;
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
    return 0;
  }

  int returnVal = count;

  xSemaphoreGiveRecursive(mutex);
  return returnVal;
}

Packet &SensorUnitReadings::getReading(PacketInfo_t packet) {
  static Packet sentinel{};
  if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
    Serial.println("Failed to take mutex");
    return sentinel;
  }

  for (uint8_t i{0}; i < count; i++) {
    if (packets[i].info == packet) {
      xSemaphoreGiveRecursive(mutex);
      return packets[i];
    }
  }

  Serial.println("Reading not found");
  xSemaphoreGiveRecursive(mutex);
  return sentinel;
}
