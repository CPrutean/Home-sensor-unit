#include <Core/global_include.h>
#include "MessageQueue.h"
/** 
@breif: MessageQueue object constructor to create internal ESP32 Queue
*/
MessageQueue::MessageQueue() {
  queueHandle = xQueueCreate(MAXQUEUELEN, sizeof(Packet));

  if (queueHandle == NULL) {
    Serial.println("FATAL: Failed to create FreeRTOS queue!");
  }
}
/** 
@breif: default destructor to delete internal queue 
*/
MessageQueue::~MessageQueue() {
  Serial.println("Destroyed MessageQueue object, deleting FreeRTOS queue.");
  if (queueHandle != NULL) {
    vQueueDelete(queueHandle);
  }
}

#define QUEUE_TIMEOUT_TICKS pdMS_TO_TICKS(100)

/** 
@breif: sends a packet to the internal ESP32 Queue
@param const Packet& packet: the packet being sent to the internal Packet queue
@return: returns if the packet was successfully sent to the queue
*/
bool MessageQueue::send(const Packet &packet) {
  return xQueueSend(queueHandle, (const void *)&packet, QUEUE_TIMEOUT_TICKS) == pdPASS;
}

/** 
@breif: recieves a packet from the internal ESP32 Queue
@param: Packet& packet: recieves a packet internal from the Packet queue
@return: returns if the packet was successfully recieved
*/
bool MessageQueue::receive(Packet &packet) {
  return xQueueReceive(queueHandle, (void *)&packet, QUEUE_TIMEOUT_TICKS) ==pdTRUE;
}
