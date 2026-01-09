#include "SensorUnitManager.h"
#include <esp_now.h>

// Single definition for the global manager pointer declared in the header
SensorUnitManager *sensUnitMngr = nullptr;

// Trivial destructor definition (was only declared in header)
SensorUnitManager::~SensorUnitManager() = default;

auto SensorUnitManager::getSensorUnitInfo(int ind) -> SensorUnitInfo& {
  return suInfo[ind];
}

void SensorUnitManager::sendToSu(const Packet &packet, int suNum) {
  esp_err_t result = esp_now_send(suInfo[suNum].peerInf.peer_addr, (uint8_t *)&packet, sizeof(packet));
  if (result != ESP_OK) {
    Serial.println("Packet failed to send");
  } else {
    Serial.println("Packet success");
  }
}

// Method callback should be light, there is nothing to be done here but ensure
// that the user can read the input
void sensUnitManagerSendCB(const uint8_t *mac, esp_now_send_status_t status) {
  if (status == ESP_OK) {
    Serial.println("Packet Success");
  } else {
    Serial.println("Packet failed");
  }
}

void sensUnitManagerRecvCB(const esp_now_recv_info_t *recvInfo, const uint8_t *data, int dataLen) {
  Packet packet{};
  memcpy(&packet, data, sizeof(Packet));
  memcpy(packet.senderAddr, recvInfo->src_addr, 6);
  if (sensUnitMngr != nullptr) {
    sensUnitMngr->msgQueue.send(packet);
  }
}

// Will only work if local PMKKEY and LMKKEY variables have been properly
// initialized
void SensorUnitManager::initESPNOW() {
  if (PMKKEY[0] == '\0') {
    Serial.println(
        "ESPNOW initalization failed due to constructor never being called");
    return;
  }

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Failure initializing ESP-NOW");
    return;
  }


  sensUnitMngr = this; //Used for initializing global callbacks

  esp_now_set_pmk((uint8_t *)PMKKEY);
  esp_now_register_recv_cb(esp_now_recv_cb_t(sensUnitManagerRecvCB));
  esp_now_register_send_cb(esp_now_send_cb_t(sensUnitManagerSendCB));

  for (auto &suInf: suInfo) {
    suInf.peerInf.channel = 0;
    suInf.peerInf.encrypt = true;
    suInf.peerInf.ifidx = WIFI_IF_STA;
    if (esp_now_add_peer(&suInf.peerInf) != ESP_OK) {
      Serial.println("Failed to add a peer please try again");
    }
  }
  Serial.println("Finished ESPNOWINIT");
}
int SensorUnitManager::macInd(const uint8_t *mac) {
  bool found = true;
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

void SensorUnitManager::handlePacket(const Packet& packet) {
  int ind = macInd(packet.senderAddr);
  if (ind == -1) {
    Serial.println("Failed to handle packet");
    return;
  }


  if (packet.type == Packet::ACK) {
    msgAck.removeAckArrItem(packet.msgID); 
    return;
  } else if (packet.info.ind == 0 && packet.info.sensor == Sensors_t::BASE && packet.dataType == Packet::STRING_T) [[unlikely]]{ //Used for initializing the sensor 
    dataConverter d;
    memcpy(d.data, packet.packetData, packet.size);
    SensorDefinition sens;
    sens.fromString(d.str, sizeof(d));
    bool found = false;
    for (int i{0}; i < suInfo[ind].sensorCount; i++) {
      if (sens.sensor == suInfo[ind].sensors[i].sensor) {
        found = true;
        break;  
      }
    }
    if (found) {
      return;
    }
    suInfo[ind].sensors[suInfo[ind].sensorCount++] = sens;

    uint8_t totalReadingCount{0};
    for (int i{0}; i < suInfo[ind].sensorCount; i++) {
      totalReadingCount += suInfo[ind].sensors[i].numValues;
    }
    suInfo[ind].readings.setReadingCount(totalReadingCount);
  } else [[likely]]{
    suInfo[ind].readings.postReading(packet.packetData, packet.size, packet.info);
  } 
}

uint8_t SensorUnitManager::getSuCount() {
  return suCount;
}



void SensorUnitManager::initSensorUnitSensors(int suIndex) {
  Packet p;
  p.info.ind = 0;
  p.info.sensor = Sensors_t::BASE;
  p.type = Packet::PING; 
  this->sendToSu(p, suIndex);
}