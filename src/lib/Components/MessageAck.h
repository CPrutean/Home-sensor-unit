#pragma once
#include <Core/global_include.h>

// Forward declaration of enum (defined in SensorUnitManager.h)
enum class SensorUnitStatus : uint8_t;

#define BEGINACKLEN 8
class MessageAck final {
public:
  MessageAck();
  void addNewAckArrItem(unsigned long long msgID, unsigned long long postTime); //For the length of the array objects
  bool removeAckArrItem(unsigned long long msgID); //Removes a ackArrItem that was issued
  void removeTimedOutReq(SensorUnitStatus* suStatus); //Provide a buffer that pastes the sensor units that are timed out
  ~MessageAck();
private:
  ackListItem *ackArr{nullptr};
  void resize();
  size_t capacity{BEGINACKLEN};
  size_t size{0};
  size_t arrSize{};
  SemaphoreHandle_t mutex{};
};
