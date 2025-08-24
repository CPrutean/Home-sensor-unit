#include <sensor_units.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>



uint8_t broadcastAddress[][6] = {{0x00, 0x4b, 0x12, 0x3e, 0x87, 0x64}};
uint8_t suCount = 1;
communication_unit CU1;
sensor_unit* sens_unit_ptr;
communication_unit* com_unit_ptr;
msg_queue *messageQueue;

void onDataSent(const uint8_t* addr, esp_now_send_status_t status) {
  #ifdef DEBUG
  Serial.println("Packet sent");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery success":"Delivery failed");
  #endif
}

void onDataRecv(const uint8_t* adr, const uint8_t* data, int len) {
  #ifdef DEBUG
  Serial.println("Message recieved");
  #endif
  def_message_struct msg;
  memcpy(&msg, data, sizeof(msg));
  memcpy(&msg.senderMac, data, 6);
  BaseType_t xHigherPriorityTaskWoken = pdPASS;
  CU1.queue->send(msg);
}


void queueHandlerTask(void* pvParameters) {
  def_message_struct msg;
  for (;;) {
    if (CU1.queue->receive(msg)) {
      handleMSG_CU(msg);
    }
  }
}


void serialHandlerTask(void* pvParameters) {
  for (;;) {
    if (Serial.available()) {
      String pyStr = Serial.readStringUntil('\n');
      int len = pyStr.length();
      char cStr[len+1];
      pyStr.toCharArray(cStr, len+1);
      cStr[len] = '\0';
      respondPiRequest(cStr);
    }
    vTaskDelay(40 / portTICK_PERIOD_MS);
  }
}

esp_now_peer_info_t peerInfo;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  messageQueue = new msg_queue();
  CU1.queue = messageQueue;
  CU1.numOfSU = 1;
  com_unit_ptr = &CU1;
  sens_unit_ptr = nullptr;

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    #ifdef DEBUG
    Serial.println("Failed to init exiting");
    #endif
    exit(-1);
  }
  int i;
  for (i = 0; i < CU1.numOfSU; i++) {
    memcpy(peerInfo.peer_addr, broadcastAddress[i], 6);
    memcpy(CU1.SU_ADDR[i], broadcastAddress[i], 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
      #ifdef DEBUG
      Serial.println("Failed to add peer exiting");
      #endif
      exit(-1);
    }
  }
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));

  xTaskCreatePinnedToCore(queueHandlerTask, "Queue Handler Task", 8192, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(serialHandlerTask, "Serial Handler Task", 8192, NULL, 1, NULL, 1);
}

void loop() {
  respondPiRequest("PULL|ALL|");
  delay(5000);
}
