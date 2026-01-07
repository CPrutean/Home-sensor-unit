#pragma once
#include <Components/MessageQueue.h>
#include <Core/global_include.h>
#include <DHT.h>
#include <PIR.h>
#include <esp_now.h>
#include <Wire.h>
// As more sensors get added certain things will be added onto but the
// SensorUnitManager should behave the exact same
class SensorUnit final {
public:
  SensorUnit(const SensorUnit &) = delete;
  void handlePacket(const Packet &packet);
  MessageQueue msgQueue{};
  void initESPNOW();
  explicit SensorUnit(uint8_t *cuMac, const char *PMKKEYIN, const char *LMKKEYIN, DHT *tempIn = nullptr, PIR *motion = nullptr);
  void sendPacket(const Packet& p);
  SensorUnit() = delete;
  friend void sendAllPackets(SensorUnit& sensUnit);
  friend void baseCommands(SensorUnit& sensUnit, Packet& p, uint8_t ind);
  DHT *temp{nullptr};
  PIR *motion{nullptr};
  void initSensorDefinition(SensorDefinition& SensorDefinition);
  dataConverter d;
private:
  esp_now_peer_info_t cuPeerInf{};
  char PMKKEY[16]{};
  char CULMKKEY[16]{};
  SensorDefinition sensorsAvlbl[static_cast<int>(Sensors_t::NUM_OF_SENSORS)]{};
  uint8_t sensCount{};
};

// Declaration only. Definition lives in a single translation unit.
extern SensorUnit *sensUnitPtr;
