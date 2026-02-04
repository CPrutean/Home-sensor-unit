#pragma once
#include <Core/global_include.h>
#include <WebServer.h>
#include <WebComponents/WebServerSUM.h>
#include <esp_now.h>
#include "ManagerTypes.h"
#include <Components/MessageQueue.h>
#include <Components/MessageAck.h>


// SensorUnitManagers are responsible for sending and receiving messages between sensor units
class SensorUnitManager final {
public:
  auto getSensorUnitInfo(int ind) -> SensorUnitInfo&;
  SensorUnitManager(const SensorUnitManager &) = delete;
  virtual ~SensorUnitManager();
  explicit SensorUnitManager(const uint8_t macAdrIn[MAXPEERS][6], uint8_t suCountIn, WebServer &serv, const char *PMKKEYIN, const char **LMKKEYSIN);
  #ifndef TESTING 
  SensorUnitManager() = delete;
  #else
  explicit SensorUnitManager();
  #endif 

  void sendToSu(const Packet &p, int suNum);
  void handlePacket(const Packet &packet);
  void initESPNOW();
  void initSensorUnitSensors(int suIndex);
  void pingAllSU();
  uint8_t getSuCount();
  MessageQueue msgQueue{};
  MessageAck msgAck{};
protected:
  SensorUnitInfo suInfo[MAXPEERS]{};
  uint8_t suCount{};
  unsigned long long msgID{};
  char PMKKEY[16]{'\0'};
  int macInd(const uint8_t *mac);
  WebServer* servPtr{nullptr};
  void serializeSensorInfo(int ind, const Packet& p);
};

void sensUnitManagerSendCB(const uint8_t *mac, esp_now_send_status_t status);
void sensUnitManagerRecvCB(const esp_now_recv_info_t *recvInfo, const uint8_t *data, int dataLen);