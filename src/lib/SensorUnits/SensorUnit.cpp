#include "SensorUnits.h"
#include <esp_now.h>

void sendAllPackets(SensorUnit& sensUnit);
void initSensUnit(SensorUnit& sensUnit);
void handlePostRequests(SensorUnit& sensUnit);
// Forward declaration to allow use before definition


// Single definition of the global pointer declared in SensorUnits.h
static SensorUnit* sensUnitPtr = nullptr;

//This is a directive which defines the method parameters 
//Defines a cmdFunc as a function which returns void and takes in the parameters of SensorUnit& Packet& and uint8_t

/** 
@breif: Default constructor for sensor units copies the essential LMKKEYS and PMKKEYS internally for encryption, the SensorUnitManager
mac address, the pointers to the Sensors, and the pointers to the motion sensors
@param: communication unit mac address in
@param: const char* PMKKEYIN: PMKKEY for standard ESP32 encryption
@param: const char* LMKKEYIN: LMKKEY for standard ESP32 encryption
@param: DHT* tempIN: Temperature sensor in if the sensor unit has one available, default param value is nullptr
@param: PIR* motionIn: Motion sensor in if the sensor unit has one available, default param value is nullptr 
*/
SensorUnit::SensorUnit(const uint8_t *cuMac, const char *PMKKEYIN, const char *LMKKEYIN, DHT *tempIn, PIR *motionIn)
: temp{tempIn}, motion{motionIn} {
  sensUnitPtr = this;
  memcpy(cuPeerInf.peer_addr, cuMac, 6);
  memcpy(cuPeerInf.lmk, LMKKEYIN, 16);
  uint8_t count{0};
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
  sensUnitPtr = this;
}


/** 
@breif: default sensor unit ESP-NOW callback for messages recieved
@param: const esp_now_recv_info_t *recvInfo: ESPIDF struct which contains destination address, sender address and RXINFO
@param: const uint8_t *data: the data recieved from ESP-NOW
@param: int dataLen: the length in bytes for  how big the packet was
*/
void sensUnitRecvCB(const esp_now_recv_info_t *recvInfo, const uint8_t *data, int dataLen) {
  //Assumes sensUnitPtr is properly initialized
  Serial.println("Recieved Packet");
  Packet p{};
  memcpy(&p, data, sizeof(Packet));
  if (sensUnitPtr != nullptr) [[likely]]{
    sensUnitPtr->msgQueue.send(p);
  } else {
    Serial.println("Failed to send to queue");
  }
}


/** 
@breif: default sensor unit ESP-NOW callback for sending messages
@param: Mac address and status of callback
*/
void sensUnitSendCB(const uint8_t *mac, esp_now_send_status_t status) {
  if (status == ESP_OK) {
    Serial.println("Packet Success");
  } else {
    Serial.println("Packet failed to send");
  }
}

/** 
@breif: initializes ESP-NOW assuming proper constructor was called
*/
void SensorUnit::initESPNOW() {
  if (PMKKEY[0] == '\0') {
    Serial.println("ESPNOW failed to intialize");
    return;
  }

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Failure initializing ESP-NOW");
    return;
  }

  esp_now_set_pmk((uint8_t *)PMKKEY);
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
@breif: sends a packet to the SensorUnitManager via ESP-NOW
@param: const Packet& p: packet to be sent
*/
void SensorUnit::sendPacket(const Packet& p) {
  esp_err_t result = esp_now_send(cuPeerInf.peer_addr, (uint8_t *)&p, sizeof(p));
  if (result != ESP_OK) {
    Serial.println("Packet failed to send");
  } else {
    Serial.println("Packet sent successfully");
  }
}

void SensorUnit::handlePacket(const Packet &p) {
  
  Packet pac{};

  if (p.type == Packet::PING) { 
    pac.type = Packet::ACK;
    pac.dataType = Packet::NULL_T;
    sendPacket(pac);
    if (p.info.sensor == Sensors_t::BASE && p.info.ind == 0) {
      baseCommands(*this, pac, 0); //Initialize sensors AKA serialize sensors and send appropriate packets
    } else [[likely]]{
      sendAllPackets(*this); //Send all reading packets on ping
    }
  } else if (p.type == Packet::POST) {
    int i{0};
    for (i = 0; i < sensCount; i++) {
      if (sensorsAvlbl[i].sensor == p.info.sensor) {
        break;
      }      
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
  } else [[unlikely]] {
    writeErrorMsg(pac, "INVALID PACKET TYPE");
    sendPacket(pac);
  }

  
}

/**
 * @breif: Sends all reading packets to the communication unit.
 * @param: Reference to the sensor unit we are sending packets from
 */
void sendAllPackets(SensorUnit& sensUnits) {
  Packet pac{};
  defaultFnMethods func;  
  for (int i{0}; i < sensUnits.sensCount-1; i++) { //Ignore the base commands
    func  = reinterpret_cast<defaultFnMethods>(sensUnits.sensorsAvlbl[i].fnMemAdr);
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





#ifdef TESTING
SensorUnit::SensorUnit() { //Create fake sensors 
  for (int i{0}; i < 3; i++) {
    sensorsAvlbl[i].sensor = static_cast<Sensors_t>(i);
    initSensorDefinition(sensorsAvlbl[i]);
  }  
}
#endif