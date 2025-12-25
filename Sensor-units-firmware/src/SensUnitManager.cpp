#include "SensorUnitManager.h"
#include "esp_now.h"

// Single definition for the global manager pointer declared in the header
SensorUnitManager *sensUnitMngr = nullptr;

// Trivial destructor definition (was only declared in header)
SensorUnitManager::~SensorUnitManager() = default;
void SensorUnitManager::sendToSu(const Packet &packet, int suNum) {
  esp_err_t result = esp_now_send(suPeerInf[suNum].peer_addr, (uint8_t *)&packet, sizeof(packet));
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

  esp_now_set_pmk((uint8_t *)PMKKEY);
  esp_now_register_recv_cb(esp_now_recv_cb_t(sensUnitManagerRecvCB));
  esp_now_register_send_cb(esp_now_send_cb_t(sensUnitManagerSendCB));

  for (esp_now_peer_info_t peerInf : suPeerInf) {
    peerInf.channel = 0;
    peerInf.encrypt = true;
    peerInf.ifidx = WIFI_IF_STA;
    if (esp_now_add_peer(&peerInf) != ESP_OK) {
      Serial.println("Failed to add a peer please try again");
    }
  }
  Serial.println("Finished ESPNOWINIT");
}
