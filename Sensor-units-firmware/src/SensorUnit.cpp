#include "SensorUnits.h"
#include "esp_now.h"
SensorUnit::SensorUnit(uint8_t *cuMac, const char *PMKKEYIN,
                       const char *LMKKEYIN, DHT *tempIn, PIR *motionIn)
    : temp{tempIn}, motion{motionIn} {
  memcpy(cuPeerInf.peer_addr, cuMac, 6);
  memcpy(cuPeerInf.lmk, LMKKEYIN, 16);
  if (temp != nullptr) {
    sensorsAvlbl[0].sensor = Sensors_t::TEMPERATURE_AND_HUMIDITY;
    initSensorDefinition(sensorsAvlbl[0]);
  }

  if (motion != nullptr) {
    sensorsAvlbl[1].sensor = Sensors_t::MOTION;
    initSensorDefinition(sensorsAvlbl[1]);
  }
  memcpy(PMKKEY, PMKKEYIN, 16);
}

void sensUnitRecvCB(const esp_now_recv_info_t *recvInfo, const uint8_t *data,
                    int dataLen) {
  if (sensUnitPtr == nullptr) {
    Serial.println("sensUnitPtr was never initialized");
    return;
  }
  Packet p{};
  memcpy(&p, data, sizeof(Packet));
  sensUnitPtr->msgQueue.send(p);
}

void sensUnitSendCB(const uint8_t *mac, esp_now_send_status_t status) {
  if (status == ESP_OK) {
    Serial.println("Packet Success");
  } else {
    Serial.println("Packet failed to send");
  }
}

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

void SensorUnit::handlePacket(const Packet &p) {}
