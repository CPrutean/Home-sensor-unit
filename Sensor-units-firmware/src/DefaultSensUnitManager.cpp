#include "SensorUnitManager.h"
#include "lwipopts.h"
void SensorUnitManager::sendToSu(Packet packet, int suNum) {
  esp_err_t result =
      esp_now_send(m_suMac[suNum], (uint8_t *)&packet, sizeof(packet));
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

void sensUnitManagerRecvCB(const esp_now_recv_info_t *recvInfo,
                           const uint8_t *data, int dataLen) {
  Packet packet{};
  memcpy(&packet, data, sizeof(Packet));

  sensUnitMngr->msgQueue.send(packet);
}

void SensorUnitManager::initESPNOW() {
  if (suPeerInf[0].peer_addr[0] == 0) {
    Serial.println("FAILED TO INIT, CONSTRUCTOR NEVER CALLED");
    return;
  }
  Serial.begin(115200);
}
