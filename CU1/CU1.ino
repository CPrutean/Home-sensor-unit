#include <sensor_units.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#define PI_SERIAL Serial

uint8_t broadcastAddress[][6] = {0x3c, 0x8a, 0x1f, 0xd5, 0x44, 0xf8};
uint8_t suCount = 1;
communication_unit CU1;
sensor_unit* sens_unit_ptr;
communication_unit* com_unit_ptr;
msg_queue messageQueue;

void queueHandlerTask(void* pvParameters) {
  for (;;) {
    if (!CU1.queue->isEmpty() && CU1.queue != nullptr) {
      def_message_struct msg;
      msg = CU1.queue->getFront();

      def_message_struct response;
      memset(&response, 0, sizeof(response));
      response.message[0]  = '\0';

      handleMSG_CU(msg, msg.channel);
      CU1.queue->pop();
    }
    if (PI_SERIAL.available()) {
      String pyStr = PI_SERIAL.readStringUntil('\n');
      int len = pyStr.length();
      char cStr[len+1];
      pyStr.toCharArray(cStr, len+1);
      cStr[len] = '\0';
      respondPiRequest(cStr);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  CU1.queue = &messageQueue;
  com_unit_ptr = &CU1;
  sens_unit_ptr = nullptr;
  int i;
  int j;
  for (i = 0; i < suCount; i++) {
    for (j = 0; j < 6; j++) {
      CU1.SU_ADDR[i][j] = broadcastAddress[i][j];
    }
  }
  init_CU_ESPNOW(com_unit_ptr);
  xTaskCreatePinnedToCore(queueHandlerTask, "Queue Handler Task", 8192, NULL, 1, NULL, 1);
}

void loop() {
  // put your main code here, to run repeatedly:

}
