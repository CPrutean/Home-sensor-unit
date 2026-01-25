#pragma once
#include <Core/global_include.h>
#define MAXQUEUELEN 32
class MessageQueue {
public:
  bool send(const Packet &packet);
  bool receive(Packet &packet);
  QueueHandle_t getQueueHandle() const;
  explicit MessageQueue();
  ~MessageQueue();

private:
  QueueHandle_t queueHandle;
};
