#include <esp_now.h>

#include "Core/global_include.h"
#include "SensorUnits.h"

void sendAllPackets(SensorUnit& sensUnit);
void initSensUnit(SensorUnit& sensUnit);
void handlePostRequests(SensorUnit& sensUnit);
// Forward declaration to allow use before definition

// Single definition of the global pointer declared in SensorUnits.h
static SensorUnit* sensUnitPtr = nullptr;

// This is a directive which defines the method parameters
// Defines a cmdFunc as a function which returns void and takes in the
// parameters of SensorUnit& Packet& and uint8_t

/**
@brief: Default constructor for sensor units copies the essential LMKKEYS and
PMKKEYS internally for encryption, the SensorUnitManager mac address, the
pointers to the Sensors, and the pointers to the motion sensors
@param: communication unit mac address in
@param: const char* PMKKEYIN: PMKKEY for standard ESP32 encryption
@param: const char* LMKKEYIN: LMKKEY for standard ESP32 encryption
@param: DHT* tempIN: Temperature sensor in if the sensor unit has one available,
default param value is nullptr
@param: PIR* motionIn: Motion sensor in if the sensor unit has one available,
default param value is nullptr
*/
SensorUnit::SensorUnit(const uint8_t* cuMac, const char* PMKKEYIN,
                       const char* LMKKEYIN, DHT* tempIn, PIR* motionIn)
    : SensorUnit(PMKKEYIN, LMKKEYIN, tempIn, motionIn) {
  memcpy(cuPeerInf.peer_addr, cuMac, 6);
}

SensorUnit::SensorUnit(const char* PMKKEYIN, const char* LMKKEYIN, DHT* tempIn,
                       PIR* motionIN)
    : temp{tempIn}, motion{motionIN} {
  sensUnitPtr = this;
  uint8_t count{0};
  memcpy(cuPeerInf.lmk, LMKKEYIN, 16);  // Potentially in unpaired state

  if (temp != nullptr) {
    sensorsAvlbl[count].sensor = Sensors_t::TEMPERATURE_AND_HUMIDITY;
    initSensorDefinition(sensorsAvlbl[count++]);
  }

  if (motion != nullptr) {
    sensorsAvlbl[count].sensor = Sensors_t::MOTION;
    initSensorDefinition(sensorsAvlbl[count++]);
  }
  sensorsAvlbl[count].sensor = Sensors_t::BASE;
  initSensorDefinition(sensorsAvlbl[count++]);

  this->sensCount = count;
  memcpy(PMKKEY, PMKKEYIN, 16);
}

/**
@brief: default sensor unit ESP-NOW callback for messages received
@param: const esp_now_recv_info_t *recvInfo: ESPIDF struct which contains
destination address, sender address and RXINFO
@param: const uint8_t *data: the data received from ESP-NOW
@param: int dataLen: the length in bytes for  how big the packet was
*/
void sensUnitRecvCB(const esp_now_recv_info_t* recvInfo, const uint8_t* data,
                    int dataLen) {
  // Assumes sensUnitPtr is properly initialized
  Serial.println("Received Packet");
  Packet p{};
  memcpy(&p, data, sizeof(Packet));
  if (sensUnitPtr != nullptr) [[likely]] {
    sensUnitPtr->msgQueue.send(p);
  } else {
    Serial.println("Failed to send to queue");
  }
}

/**
@brief: default sensor unit ESP-NOW callback for sending messages
@param: Mac address and status of callback
*/
void sensUnitSendCB(const uint8_t* mac, esp_now_send_status_t status) {
  if (status == ESP_OK) {
    Serial.println("Packet Success");
  } else {
    Serial.println("Packet failed to send");
  }
}

/**
@brief: initializes ESP-NOW assuming proper constructor was called
*/
void SensorUnit::initESPNOW() {
  if (PMKKEY[0] == '\0') {
    Serial.println("ESPNOW failed to initialize");
    return;
  }

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Failure initializing ESP-NOW");
    return;
  }

  esp_now_set_pmk((uint8_t*)PMKKEY);
  cuPeerInf.channel = 0;
  cuPeerInf.encrypt = false;
  cuPeerInf.ifidx = WIFI_IF_STA;

  if (esp_now_add_peer(&cuPeerInf) != ESP_OK) {
    Serial.println("Failed to add SensUnitManager as a peer please try again");
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(sensUnitRecvCB));
  esp_now_register_send_cb(esp_now_send_cb_t(sensUnitSendCB));

  Serial.println("Finished initializing");
}

/**
@brief: sends a packet to the SensorUnitManager via ESP-NOW
@param: const Packet& p: packet to be sent
*/
void SensorUnit::sendPacket(Packet& p) {
  WiFi.macAddress(p.senderAddr);

  esp_err_t result = esp_now_send(cuPeerInf.peer_addr, (uint8_t*)&p, sizeof(p));
  if (result != ESP_OK) {
    Serial.println("Packet failed to send");
  } else {
    Serial.println("Packet sent successfully");
  }
}

void SensorUnit::broadcastAvailability() {
  Packet p{};
  WiFi.macAddress(p.senderAddr);
  p.type = Packet::DISCOVERY;
  // Broadcast mac address
  uint8_t mac[6]{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esp_err_t result = esp_now_send(mac, (uint8_t*)&p, sizeof(p));
  if (result != ESP_OK) {
    Serial.println("broadcast failed to send");
  } else {
    Serial.println("broadcast sent successfully");
  }
}

void SensorUnit::handlePacket(const Packet& p) {
  Packet pac{};

  if (p.type == Packet::PING) {
    pac.type = Packet::ACK;
    pac.dataType = Packet::NULL_T;
    sendPacket(pac);
    if (p.info.sensor == Sensors_t::BASE && p.info.ind == 0) {
      baseCommands(*this, pac, 0);  // Initialize sensors AKA serialize sensors
                                    // and send appropriate packets
    } else [[likely]] {
      sendAllPackets(*this);  // Send all reading packets on ping
    }
  } else if (p.type == Packet::POST) {
    int i{0};
    for (i = 0; i < sensCount; i++) {
      if (sensorsAvlbl[i].sensor == p.info.sensor) {
        break;
      }
    }

    if (i >= sensCount) {
      writeErrorMsg(pac, "SENSOR NOT FOUND");
      sendPacket(pac);
      return;
    }

    if (sensorsAvlbl[i].msgType[p.info.ind] != Packet::POST) {
      writeErrorMsg(pac, "INVALID COMMAND SENT POST VALUE WAS FALSE");
      sendPacket(pac);
      return;
    }

    defaultFnMethods func;
    func = reinterpret_cast<defaultFnMethods>(sensorsAvlbl[i].fnMemAdr);
    func(*this, pac, p.info.ind);
    sendPacket(pac);
  } else if (p.type == Packet::DISCOVERY_REQ) {
    uint8_t mac[6];
    memcpy(mac, p.senderAddr, 6);
    int channel = p.i;
    addCuPeers(mac, channel);
    Packet pac{};
    WiFi.macAddress(pac.senderAddr);
    pac.type = Packet::DISCOVERY_FIN;
    sendPacket(pac);
  } else [[unlikely]] {
    writeErrorMsg(pac, "INVALID PACKET TYPE");
    sendPacket(pac);
  }
}

/**
 * @brief: Sends all reading packets to the communication unit.
 * @param: Reference to the sensor unit we are sending packets from
 */
void sendAllPackets(SensorUnit& sensUnits) {
  Packet pac{};
  defaultFnMethods func;
  for (int i{0}; i < sensUnits.sensCount - 1;
       i++) {  // Ignore the base commands
    func =
        reinterpret_cast<defaultFnMethods>(sensUnits.sensorsAvlbl[i].fnMemAdr);
    pac.info.sensor = sensUnits.sensorsAvlbl[i].sensor;
    for (int j{0}; j < sensUnits.sensorsAvlbl[i].numValues; j++) {
      if (sensUnits.sensorsAvlbl[i].msgType[j] == Packet::READING) {
        pac.type = sensUnits.sensorsAvlbl[i].msgType[j];
        pac.info.ind = j;
        func(sensUnits, pac, j);
        sensUnits.sendPacket(pac);
      }
    }
  }
}

void SensorUnit::addCuPeers(const uint8_t* mac, int channel) {
  memcpy(cuPeerInf.peer_addr, mac, 6);
  // Assuming that the sensor unit gets the mac address normally
  // then the devices are going to be added to esp_now and start running as
  // normal
  if (esp_now_add_peer(&cuPeerInf) != ESP_OK) {
    Serial.println("Cu failed to add a peer");
  } else {
    Serial.println("Successfully added a peer");
  }
}

#ifdef TESTING
SensorUnit::SensorUnit() {  // Create fake sensors
  for (int i{0}; i < 3; i++) {
    sensorsAvlbl[i].sensor = static_cast<Sensors_t>(i);
    initSensorDefinition(sensorsAvlbl[i]);
  }
}
#endif
