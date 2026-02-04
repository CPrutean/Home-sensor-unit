#pragma once
#include <Arduino.h>
#include "../Core/global_include.h"
#include <esp_now.h>
#define MAXSENSORS 3

// Enum moved outside class to break circular dependency with MessageAck
enum class SensorUnitStatus : uint8_t {ONLINE = 0, ERROR, OFFLINE, NUMTYPES};



//Group readings by sensor unit
//Group readings internally by the sensor definition indexes and the values
class SensorUnitReadings final {
public:
  explicit SensorUnitReadings();
  void postReading(const Packet &p);
  int getReadingCount();
  void postNewSensor(Sensors_t sensor, uint8_t readingCount);
  Packet& getReading(PacketInfo_t packet);
private:
  Packet** packetGroupings{nullptr};
  uint8_t* packetCount{nullptr};
  uint8_t sensorCount{0};
  
  xSemaphoreHandle mutex = xSemaphoreCreateRecursiveMutex();
};


struct SensorUnitInfo {
  SensorDefinition sensors[MAXSENSORS]{};
  SensorUnitReadings readings{}; 
  char LMKKEY[16]{};
  esp_now_peer_info_t peerInf{};
  uint8_t sensorCount{0};
  SensorUnitStatus status{};
  unsigned long SensorUnitID{0};
};



