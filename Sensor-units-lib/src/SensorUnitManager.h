#include "MessageAck.h"
#include "MessageQueue.h"
#include "global_include.h"
#include <WiFi.h>
#include <esp_now.h>
#if __BYTE_ORDER__ == __LITTLE_ENDIAN__
#define ____BYTE_ORDER__ __ORDER_BIG__ORDER_BIG_ENDIAN__
#endif

// SensorUnitManagers are responsible for sending and receiving messages between
// sensor sensor units
class SensorUnitManager {
public:
  enum SensorUnitStatus { ONLINE = 0, ERROR, OFFLINE };
  SensorUnitManager(const SensorUnitManager &) = delete;
  virtual ~SensorUnitManager();
  SensorUnitManager(uint8_t **macAdrIn, int numOfSuIn, const char *PMKKEYIN,
                    const char *LMKKEYIN)
      : numOfSu{static_cast<size_t>(numOfSuIn)} {
    for (int i = 0; i < numOfSuIn; i++) {
      memcpy(m_suMac[i], macAdrIn[i], 6);
    }
    strncpy(PMKKEY, PMKKEYIN, 16);
    strncpy(LMKKEY, LMKKEYIN, 16);
  }
  void sendToSu(Packet packet, int suNum);
  virtual void handlePacket(const Packet &packet);
  MessageQueue msgQueue{};
  MessageAck msgAck{};
  void initESPNOW();

protected:
  uint8_t m_suMac[10][6]{};
  size_t numOfSu{};
  SensorUnitStatus m_status[10]{};
  Sensors_t sensorsAvlbl[10][3]{};
  unsigned long long msgID{};
  char PMKKEY[16]{};
  char LMKKEY[16]{};
};

class WifiSensUnitManager final : public SensorUnitManager {};

class UartSensUnitManager final : public SensorUnitManager {
private:
  void handleServerRequest(const char *buffer, size_t sizeOfBuffer);
  void printToUartBuffer(const char *buffer, size_t sizeOfBuffer);
};

extern SensorUnitManager *sensUnitMngr;

void sensUnitManagerSendCB(const uint8_t *mac, esp_now_send_status_t status);
void sensUnitManagerRecvCB(const esp_now_recv_info_t *recvInfo,
                           const uint8_t *data, int dataLen);
