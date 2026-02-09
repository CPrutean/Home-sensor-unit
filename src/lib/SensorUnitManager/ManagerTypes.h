#pragma once
#include <Arduino.h>
#include "../Core/global_include.h"
#include <esp_now.h>
#define MAXSENSORS 3
#define MAXREADINGS static_cast<uint8_t>(Sensors_t::BASE)*5
// Enum moved outside class to break circular dependency with MessageAck
enum class SensorUnitStatus : uint8_t {ONLINE = 0, ERROR, OFFLINE, NUMTYPES};



//Stores all sensor readings in a single contiguous array
//Readings are identified and looked up by their PacketInfo_t header
class SensorUnitReadings final {
public:
  explicit SensorUnitReadings() = default;
  void postReading(const Packet &p);
  int getReadingCount();
  Packet& getReading(PacketInfo_t packet);
private:
  Packet packets[MAXREADINGS]{};
  uint8_t count{0};
  uint8_t size{8};

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



