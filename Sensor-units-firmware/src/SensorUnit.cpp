#include "SensorUnits.h"
#include "esp_now.h"

void sendAllPackets(SensorUnit& sensUnit);
void tempCommands(SensorUnit& sensUnit, Packet& p, uint8_t ind);
void motionCommands(SensorUnit& sensUnit, Packet& p, uint8_t ind);
void baseCommands(SensorUnit& sensUnit, Packet& p, uint8_t ind);
void initSensUnit(SensorUnit& sensUnit);
void handlePostRequests(SensorUnit& sensUnit);

using cmdFunc = void(*)(SensorUnit&,Packet& p,uint8_t); //Defines the commandFunc for void pointer syntax

/*
@breif: initializes the internal SensorDefinition as well as the internal function pointer to handle commands requested
@param SensorDefinition &sensorDef: SensorDefinition to be assigned
*/
void SensorUnit::initSensorDefinition(SensorDefinition &sensorDef) {
  if (sensorDef.sensor == Sensors_t::NUM_OF_SENSORS) {
    Serial.println("Invalid object passed initialize a valid sensor");
    return;
  }
  // DEFINE NEW SENSORS HERE
  switch (sensorDef.sensor) {
  case (Sensors_t::TEMPERATURE_AND_HUMIDITY):
    snprintf(sensorDef.name, sizeof(sensorDef.name), "%s", "TEMPERATURE_AND_HUMIDITY");
    snprintf(sensorDef.readingStringsArray[0], sizeof(sensorDef.readingStringsArray[0]), "%s", "TEMPERATURE");
    snprintf(sensorDef.readingStringsArray[1], sizeof(sensorDef.readingStringsArray[1]), "%s", "HUMIDITY");
    sensorDef.numValues = 2;
    break;
  case (Sensors_t::MOTION):
    snprintf(sensorDef.name, sizeof(sensorDef.name), "%s", "MOTION_SENSOR");
    snprintf(sensorDef.readingStringsArray[0], sizeof(sensorDef.readingStringsArray[0]), "%s", "MOTION");
    sensorDef.numValues = 1;
    break;
  case (Sensors_t::BASE):
    snprintf(sensorDef.name, sizeof(sensorDef.name), "%s", "BASE");
    snprintf(sensorDef.readingStringsArray[0], sizeof(sensorDef.readingStringsArray[0]), "%s", "INIT");
    sensorDef.numValues = 1;
    break;
  default:
    Serial.println("Failed to init");
    break;
  };
}


/*
@breif: Default constructor for sensor units copies the essential LMKKEYS and PMKKEYS internally for encryption, the SensorUnitManager
mac address, the pointers to the Sensors, and the pointers to the motion sensors
@param uint8_t* cuMac: pointer to the array that contains the SensorUnitManager mac address
@param const char* PMKKEYIN: PMKKEY for standard ESP32 encryption
@param const char* LMKKEYIN: LMKKEY for standard ESP32 encryption
@param DHT* tempIN: Temperature sensor in if the sensor unit has one available, default param value is nullptr
@param PIR* motionIn: Motion sensor in if the sensor unit has one available, default param value is nullptr 
*/
SensorUnit::SensorUnit(uint8_t *cuMac, const char *PMKKEYIN, const char *LMKKEYIN, DHT *tempIn, PIR *motionIn)
: temp{tempIn}, motion{motionIn} {
  memcpy(cuPeerInf.peer_addr, cuMac, 6);
  memcpy(cuPeerInf.lmk, LMKKEYIN, 16);
  uint8_t count{0};
  if (temp != nullptr) {
    sensorsAvlbl[count++].sensor = Sensors_t::TEMPERATURE_AND_HUMIDITY;
    initSensorDefinition(sensorsAvlbl[0]);
  }

  if (motion != nullptr) {
    sensorsAvlbl[count++].sensor = Sensors_t::MOTION;
    initSensorDefinition(sensorsAvlbl[1]);
  }
  sensorsAvlbl[count] = NULL;
  memcpy(PMKKEY, PMKKEYIN, 16);
}


/*
@breif: default sensor unit ESP-NOW callback for messages recieved
@param const esp_now_recv_info_t *recvInfo: ESPIDF struct which contains destination address, sender address and RXINFO
@param const uint8_t *data: the data recieved from ESP-NOW
@param int dataLen: the length in bytes for  how big the packet was
*/
void sensUnitRecvCB(const esp_now_recv_info_t *recvInfo, const uint8_t *data, int dataLen) {
  //Assumes sensUnitPtr is properly initialized
  Packet p{};
  memcpy(&p, data, sizeof(Packet));
  sensUnitPtr->msgQueue.send(p);
}


/*
@breif: default sensor unit ESP-NOW callback for sending messages

*/
void sensUnitSendCB(const uint8_t *mac, esp_now_send_status_t status) {
  if (status == ESP_OK) {
    Serial.println("Packet Success");
  } else {
    Serial.println("Packet failed to send");
  }
}

/*
@breif: initializes ESP-NOW assuming proper constructor was called
*/
void SensorUnit::initESPNOW() {
  if (PMKKEY[0] = '\0') {
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
  esp_now_register_recv_cb(esp_now_recv_cb_t(sensUnitRecvCB));
  esp_now_register_send_cb(esp_now_send_cb_t(sensUnitSendCB));

  cuPeerInf.channel = 0;
  cuPeerInf.encrypt = true;
  cuPeerInf.ifidx = WIFI_IF_STA;

  if (esp_now_add_peer(&cuPeerInf)) {
    Serial.println("Failed to add SensUnitManager as a peer please try again");
  }

  Serial.println("Finished initializing");
}



void SensorUnit::handlePacket(const Packet &p) {
  Packet pac{};
  pac.msgID = p.msgID;
  pac.type = Packet::ACK;
  pac.dataType = Packet::NULL_T;
  sendPacket(pac);

  if (p.type == Packet::PING) [[likely]] {
    
  } else if (p.type == Packet::POST) {

  } else [[unlikely]] {

  }

  pac.type = Packet::FIN;
  pac.dataType = Packet::NULL_T;
  sendPacket(pac);
}


void sendAllPackets(SensorUnit& sensUnits) {

}


void tempCommands(SensorUnit& sensUnit, Packet& p, uint8_t ind) {

}

void motionCommands(SensorUnit & sensUnit, Packet& p, uint8_t ind) {

}

void baseCommands(SensorUnit& sensUnit, Packet& p, uint8_t ind) {

}
