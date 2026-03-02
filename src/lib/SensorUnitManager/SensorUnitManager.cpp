#include "SensorUnitManager.h"

#include <esp_now.h>

// Single definition for the global manager pointer declared in the header
static SensorUnitManager* sensUnitMngr = nullptr;

SensorUnitManager::SensorUnitManager(const char* PMKKEYIN,
                                     const char** LMKKEYSIN) {
  std::copy(PMKKEYIN, PMKKEYIN + 16, PMKKEY);

  for (uint8_t i{0}; i < MAXPEERS; i++) {
    std::copy(LMKKEYSIN[i], LMKKEYSIN[i] + 16, suInfo[i].peerInf.lmk);
  }
}

void SensorUnitManager::handleBringup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA);
  WiFiManager wm;
  if (!wm.autoConnect("SensorUnitManager", "Password")) {
    Serial.println("Trying to connect");
  } else {
    Serial.println("Successfully connected");
  }
  int channel{WiFi.channel()};

  initESPNOW(channel);
}
/**
 * @brief: Default constructor for the sensor unit manager,
 * which stores the mac addresses, initializes the web server pointer, and
 * stores the necessary keys for internal encryption algorithms
 * @param macAdrIn: mac addresses we are passing in
 * @param suCountIn: count of sensor units we are passing
 * @param serv: Web server object reference we are trying to pass for later
 * website functionality
 * @param PMKKEYIN: PMKKEY for encryption we are passing in
 * @param LMKKEYSIN: multiple LMKKEYS we are passing for each sensor unit
 */
SensorUnitManager::SensorUnitManager(const uint8_t macAdrIn[MAXPEERS][6],
                                     const char* PMKKEYIN,
                                     const char** LMKKEYSIN)
    : suCount{MAXPEERS} {
  sensUnitMngr = this;  // For setting up

  memcpy(PMKKEY, PMKKEYIN, 16);

  unsigned long long ID{};
  for (int i{0}; i < MAXPEERS; i++) {
    memcpy(suInfo[i].peerInf.peer_addr, macAdrIn[i], 6);
    memcpy(&ID, macAdrIn[i], 6);
    if (ID != 0) {
      suInfo[i].SensorUnitID = ID;
    } else {
      Serial.println("Invalid sensor unit info was passed");

      continue;
    }
    memcpy(suInfo[i].peerInf.lmk, LMKKEYSIN[i], 16);
    msgAck.addSensorUnit(ID);
  }
}

// Trivial destructor definition (was only declared in header)
SensorUnitManager::~SensorUnitManager() = default;

/**
 * @brief: returns information based on the sensor unit index requested
 * @param ind: index of the sensor unit we are requesting
 * @return: reference to the SensorUnitInfo
 */
auto SensorUnitManager::getSensorUnitInfo(int ind) -> SensorUnitInfo& {
  return suInfo[ind];
}

/**
 * @brief: sends packet to requested sensor unit
 * @param packet: packet we are sending to the sensor unit
 * @param suNum: sensor unit index that we are sending the message to
 */
void SensorUnitManager::sendToSu(const Packet& packet, int suNum) {
  esp_err_t result = esp_now_send(suInfo[suNum].peerInf.peer_addr,
                                  (uint8_t*)&packet, sizeof(packet));
  if (result != ESP_OK) {
    Serial.println("Packet failed to send");
  } else {
    Serial.println("Packet success");
  }
}

/**
 * @brief: default sender callback for ESP-NOW
 * @param mac: mac address of the sender
 * @param status: status of the data send
 */
void sensUnitManagerSendCB(const uint8_t* mac, esp_now_send_status_t status) {
  if (status == ESP_OK) {
    Serial.println("Packet Success");
  } else {
    Serial.println("Packet failed");
  }
}
/**
 * @brief: default message receiver callback, sends a packet to the internal
 * queue
 * @param recvInfo: esp_now struct that holds the receiver and sender address
 * @param data: data being sent to the esp-devices
 * @param datalen: size of the data being sent
 */
void sensUnitManagerRecvCB(const esp_now_recv_info_t* recvInfo,
                           const uint8_t* data, int dataLen) {
  Serial.println("Packet Received");
  Packet packet{};
  memcpy(&packet, data, sizeof(Packet));
  memcpy(packet.senderAddr, recvInfo->src_addr, 6);
  if (sensUnitMngr != nullptr) {
    sensUnitMngr->msgQueue.send(packet);
  } else {
    Serial.println("Failed to send to queue");
  }
}

/**
 * @brief: Initializes esp-now, adds Sensor unit managers as esp-now peers.
 * Initializes the PMK and LMK keys and enables encryption
 */
void SensorUnitManager::initESPNOW(int channel) {
  if (PMKKEY[0] == '\0') {
    Serial.println(
        "ESPNOW initialization failed due to constructor never being called");
    return;
  }

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Failure initializing ESP-NOW");
    return;
  }
  // set static variable to this so we can initialize callbacks
  sensUnitMngr = this;

  esp_now_set_pmk((uint8_t*)PMKKEY);
  for (int i{0}; i < suCount; i++) {
    suInfo[i].peerInf.channel = channel;
    suInfo[i].peerInf.encrypt = false;
    suInfo[i].peerInf.ifidx = WIFI_IF_STA;
    // Removed old "add a peer" logic moved to addNewSU
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(sensUnitManagerRecvCB));
  esp_now_register_send_cb(esp_now_send_cb_t(sensUnitManagerSendCB));

  Serial.println("Finished ESPNOWINIT");
}

void SensorUnitManager::addNewSU(const uint8_t* mac) {
  if (suCount >= MAXPEERS) {
    Serial.println("Too many peers cant add another one");
    return;
  }
  std::copy(mac, mac + 6, suInfo[suCount].peerInf.peer_addr);
  if (esp_now_add_peer(&suInfo[suCount].peerInf) != ESP_OK) {
    Serial.printf(
        "SensorUnitManager failed to add a new peer with mac address %d, %d, "
        "%d, %d, %d, %d\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  } else {
    Serial.println("Successfully added a peer");
    suCount++;
  }
}

/**
 * @brief: returns the index of the SensorUnit based on the mac address
 * @param mac: mac address we are trying to find the address of
 * @return: returns the index corresponding to the mac address, returns -1 if
 * the mac address isnt found
 */
int SensorUnitManager::macInd(const uint8_t* mac) {
  bool found = false;
  int i;
  for (i = 0; i < suCount; i++) {
    found = true;
    for (int j{0}; j < 6; j++) {
      if (suInfo[i].peerInf.peer_addr[j] != mac[j]) {
        found = false;
        break;
      }
    }
    if (found) {
      break;
    }
  }

  if (found) {
    return i;
  } else {
    return -1;
  }
}

/**
 * @brief: helper method to initialize sensor unit info from a packet and resize
 * internal readings object to hold new capacity for these readings
 * @param ind: index of the sensor unit we are trying to add this sensor to
 * @param packet: packet being read from
 */
void SensorUnitManager::serializeSensorInfo(int ind, const Packet& p) {
  // Create new method for this
  SensorDefinition sens;
  sens.fromString(p.str, sizeof(p.str));
  bool found = false;
  for (int i{0}; i < suInfo[ind].sensorCount; i++) {
    if (sens.sensor == suInfo[ind].sensors[i].sensor) {
      found = true;
      break;
    }
  }

  if (found) {
    Serial.println("Sensor already exists");
  } else {
    suInfo[ind].sensors[suInfo[ind].sensorCount] = sens;
    suInfo[ind].sensorCount++;
  }
}

/**
 * @brief: handles a packet received from the Sensor units
 * if it is an ACK packet then remove it from the internal MessageAck object
 * if it is a READING post it to the SensorUnitInfo readings values
 * otherwise assume packet was faulty
 *
 * @param packet: Packet we are trying to handle
 */
void SensorUnitManager::handlePacket(const Packet& packet) {
  int ind = macInd(packet.senderAddr);
  if (ind == -1 && packet.type == Packet::DISCOVERY) {
    // Then this is a new SU
    addNewSU(packet.senderAddr);
    Packet p{};
    p.type = Packet::DISCOVERY_REQ;
    p.dataType = Packet::INT_T;
    WiFi.macAddress(p.senderAddr);
    p.i = WiFi.channel();
    ind = macInd(packet.senderAddr);
    sendToSu(p, ind);
    return;
  } else if (packet.type == Packet::DISCOVERY_REQ) {
    initSensorUnitSensors(ind);
  }

  // Removed ack here since sensor units dont need ack we want ack from the
  // sensor units

  if (packet.type == Packet::ACK) {
    msgAck.packetReceived(suInfo[ind].SensorUnitID);
  } else if (packet.info.ind == 0 && packet.info.sensor == Sensors_t::BASE &&
             packet.dataType == Packet::STRING_T)
      [[unlikely]] {  // Used for initializing the sensor

    Serial.println("SERIALIZING SENSOR INFO");
    serializeSensorInfo(ind, packet);
  } else if (packet.type == Packet::READING) [[likely]] {
    Serial.println("Writing a packet");
    suInfo[ind].readings.postReading(packet);
  } else {
    Serial.println("Invalid packet type");
  }
}
/**
 *
 * @brief: returns the amount of sensor units available
 * @return: returns the number of sensor units available
 */
uint8_t SensorUnitManager::getSuCount() { return suCount; }

/**
 * @brief: Sends a packet to the sensor unit asking the sensor unit to send all
 * of the sensors it has available serialized
 * @param suIndex: index of the sensor unit we are trying to initialize
 */
void SensorUnitManager::initSensorUnitSensors(int suIndex) {
  Packet p;
  p.info.ind = 0;
  p.info.sensor = Sensors_t::BASE;
  p.type = Packet::PING;
  this->sendToSu(p, suIndex);
}

/**
 * @brief: Pings all sensor units for their readings, then implements a
 *
 */
void SensorUnitManager::pingAllSU() {
  Packet p{};
  p.info.ind = 1;
  p.type = Packet::PING;
  p.dataType = Packet::NULL_T;
  for (int i{0}; i < suCount; i++) {
    msgAck.expectPacket(suInfo[i].SensorUnitID);
    sendToSu(p, i);
  }
}
