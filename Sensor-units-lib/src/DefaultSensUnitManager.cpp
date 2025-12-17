#include "SensorUnitManager.h"
void SensorUnitManager::sendToSu(Packet packet, int suNum) {
  esp_err_t result =
      esp_now_send(m_suMac[suNum], (uint8_t *)&packet, sizeof(packet));
  if (result != ESP_OK) {
    Serial.println("Packet failed to send");
  } else {
    Serial.println("Packet success");
  }
}

void sensUnitManagerSendCB(const uint8_t *mac, esp_now_send_status_t status) {}

void sensUnitManagerRecvCB(const esp_now_recv_info_t *recvInfo,
                           const uint8_t *data, int dataLen) {
  Packet packet{};
  memcpy(&packet, data, sizeof(Packet));

  sensUnitMngr->msgQueue.send(packet);
}

void initESPNOW() {}
