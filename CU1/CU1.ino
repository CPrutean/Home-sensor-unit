#include <sensor_units.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <configuration.h>


uint8_t suCount = 1;
communication_unit CU1;
sensor_unit* sens_unit_ptr;
communication_unit* com_unit_ptr;
msg_queue *messageQueue;
messageAcknowledge *acknowledge;

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
  if (CU1.ack->removedFromFailed(msg.msgID) || CU1.ack->removeFromWaiting(msg.msgID)) {
    #ifdef DEBUG
    Serial.println("Message succesfully removed from waiting methods ");
    #endif
  }
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

void retryTask(void* pvParameters) {
  for (;;) {
    #ifdef DEBUG
    Serial.println("Running retry task"); 
    #endif
    CU1.ack->moveAllDelayedInWaiting();
    if(!CU1.ack->isFailedEmpty()) {
      #ifdef DEBUG
      Serial.println("Failed wasnt empty attempting to resend");
      #endif
      CU1.ack->retryInFailed();
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

esp_now_peer_info_t peerInfo;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  messageQueue = new msg_queue();
  acknowledge = new messageAcknowledge();
  CU1.queue = messageQueue;
  CU1.numOfSU = 1;
  CU1.ack = acknowledge;
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
    memcpy(peerInfo.peer_addr, sensor_unit_addr[i], 6);
    memcpy(CU1.SU_ADDR[i], sensor_unit_addr[i], 6);
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

  xTaskCreatePinnedToCore(retryTask, "retryFailedMessages", 8192, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(queueHandlerTask, "Queue Handler Task", 8192, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(serialHandlerTask, "Serial Handler Task", 8192, NULL, 1, NULL, 1);
}

void loop() {
  
}
