#include "MessageQueue.h"
#include "freertos/idf_additions.h"
#include <FreeRTOS.h>
#include <queue.h>

MessageQueue::MessageQueue() {
  queueHandle = xQueueCreate(MAXQUEUELEN, sizeof(Packet));

  if (queueHandle == NULL) {
    Serial.println("FATAL: Failed to create FreeRTOS queue!");
  }
}

MessageQueue::~MessageQueue() {
  Serial.println("Destroyed MessageQueue object, deleting FreeRTOS queue.");
  if (queueHandle != NULL) {
    vQueueDelete(queueHandle);
  }
}

#define QUEUE_TIMEOUT_TICKS pdMS_TO_TICKS(100)

bool MessageQueue::send(const Packet &packet) {
  return xQueueSend(queueHandle, (const void *)&packet, QUEUE_TIMEOUT_TICKS) ==
         pdPASS;
}

bool MessageQueue::receive(Packet &packet) {
  return xQueueReceive(queueHandle, (void *)&packet, QUEUE_TIMEOUT_TICKS) ==
         pdTRUE;
}
