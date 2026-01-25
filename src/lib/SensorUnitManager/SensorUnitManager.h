#pragma once
#include <Core/global_include.h>
#include <WebServer.h>
#include <WebComponents/WebServerSUM.h>
#include <esp_now.h>

#define MAXPEERS 6
#define MAXSENSORS 3

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

// Enum moved outside class to break circular dependency with MessageAck
enum class SensorUnitStatus : uint8_t {ONLINE = 0, ERROR, OFFLINE, NUMTYPES};

// Include MessageQueue and MessageAck after SensorUnitStatus is defined
#include <Components/MessageQueue.h>
#include <Components/MessageAck.h>

// SensorUnitManagers are responsible for sending and receiving messages between sensor units
class SensorUnitManager final {
public:

  struct SensorUnitInfo {
    SensorDefinition sensors[MAXSENSORS]{};
    SensorUnitReadings readings{}; 
    char LMKKEY[16]{};
    esp_now_peer_info_t peerInf{};
    uint8_t sensorCount{0};
    SensorUnitStatus status{};
  };

  auto getSensorUnitInfo(int ind) -> SensorUnitInfo&;
  SensorUnitManager(const SensorUnitManager &) = delete;
  virtual ~SensorUnitManager();
  explicit SensorUnitManager(const uint8_t macAdrIn[MAXPEERS][6], uint8_t suCountIn, WebServer &serv, const char *PMKKEYIN, const char **LMKKEYSIN);
  SensorUnitManager() = delete;
  void sendToSu(const Packet &p, int suNum);
  void handlePacket(const Packet &packet);
  void initESPNOW();
  void initSensorUnitSensors(int suIndex);
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
};

void sensUnitManagerSendCB(const uint8_t *mac, esp_now_send_status_t status);
void sensUnitManagerRecvCB(const esp_now_recv_info_t *recvInfo, const uint8_t *data, int dataLen);