#pragma once
#include <Core/global_include.h>

// Forward declaration of enum (defined in SensorUnitManager.h)
enum class SensorUnitStatus : uint8_t;

#define BEGINACKLEN 8
class MessageAck final {
public:
  MessageAck();
  void addSensorUnit(unsigned long suID);
  void expectPacket(unsigned long suID); // Tells the MessageAck that the sensor
                                         // unit manager expects a response back
  void
  packetRecived(unsigned long suID); // Tells the MessageAck we recived a packet
  double getPacketDropPercentage(
      unsigned long suID); // Returns the success rate of packet requests.
  ~MessageAck();

private:
  SemaphoreHandle_t mutex = xSemaphoreCreateRecursiveMutex();
  std::array<unsigned long, MAXPEERS> idArray{0};
  std::array<unsigned long, MAXPEERS> packetsRequested{0};
  std::array<unsigned long, MAXPEERS> packetsRecieved{0};
  uint8_t getSuArrInd(unsigned long id);
  uint8_t suCount{0};
};
