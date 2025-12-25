#pragma once
#include "MessageAck.h"
#include "MessageQueue.h"
#include "global_include.h"
#include <esp_now.h>

#define MAXPEERS 6

// SensorUnitManagers are responsible for sending and receiving messages between sensor units
class SensorUnitManager final {
public:
  enum SensorUnitStatus:uint8_t{ONLINE, ERROR, OFFLINE, NUMTYPES};
  SensorUnitManager(const SensorUnitManager &) = delete;
  virtual ~SensorUnitManager();
  explicit SensorUnitManager(uint8_t **macAdrIn, size_t numOfSuIn, const char *PMKKEYIN, const char **LMKKEYSIN)
  : numOfSu{numOfSuIn} {

    memset(suPeerInf, 0, sizeof(suPeerInf));
    for (int i = 0; i < numOfSuIn; i++) {
      memcpy(suPeerInf[i].peer_addr, macAdrIn[i], 6);
      suPeerInf[i].encrypt = true;
      memcpy(suPeerInf[i].lmk, LMKKEYSIN[i], 16);
    }
    strncpy(PMKKEY, PMKKEYIN, 16);
  }
  SensorUnitManager() = delete;
  void sendToSu(const Packet &p, int suNum);
  void handlePacket(const Packet &packet);
  MessageQueue msgQueue{};
  MessageAck msgAck{};
  void initESPNOW();
protected:
  esp_now_peer_info_t suPeerInf[MAXPEERS]{};
  SensorUnitStatus suStatus[MAXPEERS]{SensorUnitManager::NUMTYPES};
  size_t numOfSu{};
  Sensors_t sensorsAvlbl[MAXPEERS][3]{};
  unsigned long long msgID{};
  char PMKKEY[16]{'\0'};
  char LMKKEYS[MAXPEERS][16]{'\0'};
};
extern SensorUnitManager *sensUnitMngr;

void sensUnitManagerSendCB(const uint8_t *mac, esp_now_send_status_t status);
void sensUnitManagerRecvCB(const esp_now_recv_info_t *recvInfo, const uint8_t *data, int dataLen);
