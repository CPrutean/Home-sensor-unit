
#include "MessageAck.h"
#include "MessageQueue.h"
#include "global_include.h"
#include <WebServer.h>
#include "WebServerSUM.h"
#include <esp_now.h>

#define MAXPEERS 6 

//Group readings by sensor unit
//Group readings internally by the sensor definition indexes and the values
class SensorUnitReadings final {
public:
  explicit SensorUnitReadings();
  //Information will be stored as raw bytes until we have need it
  void postReading(const uint8_t *data, uint8_t sizeOfReading, PacketInfo_t readingInfo);
  int getNumOfReadings();
  void setNumOfReadings(int numOfReadings); //Will also resize the internal arrays to accomodate for size if need be
private:
  //All should be treated as arrays
  uint8_t numOfReadings{0}; //Treated as the length
  uint8_t **readings{nullptr};
  uint8_t *sizeOfReadings{nullptr};  //Treated as the length of the sub arrays of the readings 
  PacketInfo_t *readingInfo{nullptr}; //Info of the sensor and the reading index 
  xSemaphoreHandle mutex;
};

// SensorUnitManagers are responsible for sending and receiving messages between sensor units
class SensorUnitManager final {
public:
  enum SensorUnitStatus:uint8_t{ONLINE, ERROR, OFFLINE, NUMTYPES};
  SensorUnitManager(const SensorUnitManager &) = delete;
  virtual ~SensorUnitManager();
  explicit SensorUnitManager(uint8_t **macAdrIn, size_t suCountIn, WebServer &serv, const char *PMKKEYIN, const char **LMKKEYSIN)
  : suCount{suCountIn} {
    memset(suPeerInf, 0, sizeof(suPeerInf));
    for (int i = 0; i < suCountIn; i++) {
      memcpy(suPeerInf[i].peer_addr, macAdrIn[i], 6);
      suPeerInf[i].encrypt = true;
      memcpy(suPeerInf[i].lmk, LMKKEYSIN[i], 16);
    }
    strncpy(PMKKEY, PMKKEYIN, 16);
    readingsArr = new SensorUnitReadings[suCountIn]; //Will be initialized as objects populate once we recieve all of the sensor units
    servPtr = &serv;
  }


  SensorUnitReadings *readingsArr{nullptr};
  SensorUnitManager() = delete;
  void sendToSu(const Packet &p, int suNum);
  void handlePacket(const Packet &packet);
  MessageQueue msgQueue{};
  MessageAck msgAck{};
  void initESPNOW();
  SensorUnitStatus suStatus[MAXPEERS]{SensorUnitManager::NUMTYPES};
  uint8_t sensorCount[MAXPEERS]{};
  SensorDefinition sensors[MAXPEERS][3]{};
protected:
  esp_now_peer_info_t suPeerInf[MAXPEERS]{};
  uint8_t suCount{};
  unsigned long long msgID{};
  char PMKKEY[16]{'\0'};
  char LMKKEYS[MAXPEERS][16]{'\0'};
  int macInd(const uint8_t *mac);
  WebServer* servPtr{nullptr};
};
extern SensorUnitManager *sensUnitMngr;

void sensUnitManagerSendCB(const uint8_t *mac, esp_now_send_status_t status);
void sensUnitManagerRecvCB(const esp_now_recv_info_t *recvInfo, const uint8_t *data, int dataLen);