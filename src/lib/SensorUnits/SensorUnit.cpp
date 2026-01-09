#include "SensorUnits.h"
#include <esp_now.h>

void sendAllPackets(SensorUnit& sensUnit);
void tempCommands(SensorUnit& sensUnit, Packet& p, uint8_t ind);
void motionCommands(SensorUnit& sensUnit, Packet& p, uint8_t ind);
void baseCommands(SensorUnit& sensUnit, Packet& p, uint8_t ind);
void initSensUnit(SensorUnit& sensUnit);
void handlePostRequests(SensorUnit& sensUnit);
// Forward declaration to allow use before definition
void writeErrorMsg(Packet& p, dataConverter& d , std::string_view errormsg);

using defaultFnMethods = void(*)(SensorUnit&, Packet& p, uint8_t);

// Single definition of the global pointer declared in SensorUnits.h
SensorUnit* sensUnitPtr = nullptr;

//This is a directive which defines the method parameters 
//Defines a cmdFunc as a function which returns void and takes in the parameters of SensorUnit& Packet& and uint8_t

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
    sensorDef.msgType[0] = Packet::READING;
    snprintf(sensorDef.readingStringsArray[1], sizeof(sensorDef.readingStringsArray[1]), "%s", "HUMIDITY");
    sensorDef.msgType[1] = Packet::READING;

    sensorDef.fnMemAdr = reinterpret_cast<void*>(tempCommands);
    sensorDef.numValues = 2;

    break;
  case (Sensors_t::MOTION):
    snprintf(sensorDef.name, sizeof(sensorDef.name), "%s", "MOTION_SENSOR");

    snprintf(sensorDef.readingStringsArray[0], sizeof(sensorDef.readingStringsArray[0]), "%s", "MOTION");
    sensorDef.msgType[0] = Packet::READING;

    sensorDef.fnMemAdr = reinterpret_cast<void*>(motionCommands);
    sensorDef.numValues = 1;
    break;
  case (Sensors_t::BASE):
    snprintf(sensorDef.name, sizeof(sensorDef.name), "%s", "BASE");

    snprintf(sensorDef.readingStringsArray[0], sizeof(sensorDef.readingStringsArray[0]), "%s", "INIT");
    sensorDef.msgType[0] = Packet::READING;
    
    snprintf(sensorDef.readingStringsArray[1], sizeof(sensorDef.name), "%s", "PING");
    sensorDef.msgType[1] = Packet::READING;

    snprintf(sensorDef.readingStringsArray[2], sizeof(sensorDef.readingStringsArray[2]), "%s", "ERROR"); //Sent to sensorUnitManagers when something erroneous has happened
    sensorDef.msgType[2] = Packet::READING;
    //Should always be sent with an error code
    sensorDef.fnMemAdr = reinterpret_cast<void*>(baseCommands);
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
SensorUnit::SensorUnit(const uint8_t *cuMac, const char *PMKKEYIN, const char *LMKKEYIN, DHT *tempIn, PIR *motionIn)
: temp{tempIn}, motion{motionIn} {
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
  sensCount = count;
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
  if (sensUnitPtr != nullptr) {
    sensUnitPtr->msgQueue.send(p);
  }
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
  esp_now_register_recv_cb(esp_now_recv_cb_t(sensUnitRecvCB));
  esp_now_register_send_cb(esp_now_send_cb_t(sensUnitSendCB));

  cuPeerInf.channel = 0;
  cuPeerInf.encrypt = true;
  cuPeerInf.ifidx = WIFI_IF_STA;

  if (esp_now_add_peer(&cuPeerInf) != ESP_OK) {
    Serial.println("Failed to add SensUnitManager as a peer please try again");
  }

  Serial.println("Finished initializing");
}

/*
@breif: sends a packet to the SensorUnitManager via ESP-NOW
@param const Packet& p: packet to be sent
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
  pac.msgID = p.msgID;
  pac.type = Packet::ACK;
  pac.dataType = Packet::NULL_T;
  sendPacket(pac);

  if (p.type == Packet::PING && p.info.sensor == Sensors_t::BASE && p.info.ind == 0) { //Used for initialization of a SensorUnitManager
    baseCommands(*this, pac, 0);
  } else if (p.type == Packet::PING) [[likely]] {
    sendAllPackets(*this);
  } else if (p.type == Packet::POST) {

    int i{0};
    for (i = 0; i < sensCount; i++) {
      if (sensorsAvlbl[i].sensor == p.info.sensor) {
        break;
      }      
    }

    dataConverter d; // used for error reporting
    if (sensorsAvlbl[i].msgType[p.info.ind] != Packet::POST) {
      writeErrorMsg(pac, d, "INVALID COMMAND SENT POST VALUE WAS FALSE");
      sendPacket(pac);
      return;
    }

    defaultFnMethods func;
    func = reinterpret_cast<defaultFnMethods>(sensorsAvlbl[i].fnMemAdr);
    func(*this, pac, p.info.ind);
    sendPacket(pac);
  } else [[unlikely]] {
    dataConverter d; // used for error reporting
    writeErrorMsg(pac, d, "INVALID PACKET TYPE");
    sendPacket(pac);
  }

  pac.type = Packet::FIN;
  pac.dataType = Packet::NULL_T;
  sendPacket(pac);
}


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


void tempCommands(SensorUnit& sensUnit, Packet& p, uint8_t ind) {
  dataConverter &d{sensUnit.d};
  if (sensUnit.temp == nullptr) {
    writeErrorMsg(p, d, "INVALID SENSOR POINTER");
    return;
  }

  if (ind == 0) {
    d.f = sensUnit.temp->readTemperature();
    p.writeToPacket(d, sizeof(float));
    p.dataType = Packet::FLOAT_T;
    p.info.ind = ind;
  } else if (ind == 1) {
    d.f = sensUnit.temp->readHumidity();
    p.writeToPacket(d, sizeof(float));
    p.dataType = Packet::FLOAT_T;
    p.info.ind = ind;
  } else {
    writeErrorMsg(p, d, "INVALID INDEX PASSED");
  }
}

void motionCommands(SensorUnit & sensUnit, Packet& p, uint8_t ind) {
  p.type = Packet::READING;
  p.info.sensor = Sensors_t::MOTION;
  dataConverter &d{sensUnit.d};
  if (sensUnit.motion == nullptr) {
    writeErrorMsg(p, d, "SENSOR POINTER WAS NEVER INITIALIZED");
    return;
  }

  if (ind == 0) {
    d.i = sensUnit.motion->read();
    p.writeToPacket(d, sizeof(int));
  } else {
    writeErrorMsg(p, d, "INVALID INDEX PASSED");
  }
}

void baseCommands(SensorUnit& sensUnit, Packet& p, uint8_t ind) {
  dataConverter d;
  if (ind == 0) {
    int i{0};
    for (i = 0; i < sensUnit.sensCount; i++) {
      d.str[0] = '\0';
      sensUnit.sensorsAvlbl[i].toString(d.str, sizeof(d.str));
      p.writeToPacket(d, sizeof(d));
      sensUnit.sendPacket(p);
    }
  } else {
    writeErrorMsg(p, d, "INVALID IND");
  }
}


void writeErrorMsg(Packet& p, dataConverter& d , std::string_view errormsg) {
  p.info.sensor = Sensors_t::BASE;
  p.info.ind = 1;
  p.dataType = Packet::STRING_T;
  snprintf(d.str, sizeof(d.str), "%s", errormsg); 
  p.writeToPacket(d, errormsg.length());
}
