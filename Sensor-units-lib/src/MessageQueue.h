#include "global_include.h"
#include <Arduino.h>
#define MAXQUEUELEN 32
class MessageQueue {
public:
  bool send(const Packet &packet);
  bool receive(Packet &packet);
  QueueHandle_t getQueueHandle() const;
  MessageQueue();
  ~MessageQueue();

private:
  QueueHandle_t queueHandle;
};
