#pragma once
#include <Components/MessageQueue.h>
#include <Core/global_include.h>
#include <DHT.h>
#include <PIR.h>
#include <Wire.h>
#include <esp_now.h>

// As more sensors get added certain things will be added onto but the
// SensorUnitManager should behave the exact same
#define MAXSENSORS 3
class SensorUnit final {
public:
  SensorUnit(const SensorUnit &) = delete;
  void handlePacket(const Packet &packet);
  MessageQueue msgQueue{};
  void initESPNOW();
  explicit SensorUnit(const uint8_t *cuMac, const char *PMKKEYIN,
                      const char *LMKKEYIN, DHT *tempIn = nullptr,
                      PIR *motion = nullptr);
  void sendPacket(Packet &p);

#ifndef TESTING
  SensorUnit() = delete;
#else
  explicit SensorUnit();
#endif
  friend void sendAllPackets(
      SensorUnit &sensUnit); // Outside of these 2 friend methods, no sensors
                             // should have access to internal private members
  friend void baseCommands(SensorUnit &sensUnit, Packet &p, uint8_t ind);
  DHT *temp{nullptr};
  PIR *motion{nullptr};

private:
  esp_now_peer_info_t cuPeerInf{};
  char PMKKEY[16]{};
  char CULMKKEY[16]{};
  SensorDefinition sensorsAvlbl[MAXSENSORS]{};
  uint8_t sensCount{};
};

using defaultFnMethods = void (*)(SensorUnit &sensorPtr, Packet &p, uint8_t);

void writeErrorMsg(Packet &p, const char *errormsg);
void initSensorDefinition(SensorDefinition &SensorDefinition);
void tempCommands(SensorUnit &sensUnit, Packet &p, uint8_t ind);
void motionCommands(SensorUnit &sensUnit, Packet &p, uint8_t ind);
void baseCommands(SensorUnit &, Packet &p, uint8_t ind);
