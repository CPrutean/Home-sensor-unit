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
  //Information will be stored as raw bytes until we have need it
  void postReading(const uint8_t *data, uint8_t readingSize, PacketInfo_t readingInfo);
  int getReadingCount();
  void setReadingCount(int readingCount); //Will also resize the internal arrays to accomodate for size if need be
  void getReading(uint8_t *buffer, uint8_t bufferSize, PacketInfo_t packet);
private:
  //All should be treated as arrays
  uint8_t readingCount{0}; //Treated as the length
  uint8_t **readings{nullptr};
  uint8_t *readingSizeArray{nullptr};  //Treated as the length of the sub arrays of the readings
  PacketInfo_t *readingInfo{nullptr}; //Info of the sensor and the reading index
  uint8_t readingCapacity{0};
  xSemaphoreHandle mutex;
};


struct SensorUnitInfo {
  SensorDefinition sensors[MAXSENSORS]{};
  SensorUnitReadings readings(); 
  char LMKKEY[16]{};
  esp_now_peer_info_t peerInf{};
  uint8_t sensorCount{0};
  SensorUnitStatus status{};
};



