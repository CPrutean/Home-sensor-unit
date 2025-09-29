#include <LCD_I2C.h>
#include <PIR.h>
#include <sensor_units.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <configuration.h>


#define MOTION_SENSOR_PIN 34
#define SDA_PIN 22
#define SCL_PIN 21

sensor_unit SU2;
msg_queue *q = new msg_queue();

LCD_I2C lcd(0x27, 16, 2);
PIR motion;
sensor_type module[] = {MOTION_SENSOR};
sensor_unit* sens_unit_ptr;


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
  if (SU2.queue->send(msg)) {
    #ifdef DEBUG
    Serial.println("Message succesfully sent to queue");
    #endif
  } else {
    #ifdef DEBUG
    Serial.println("The message failed to be added to queue");
    #endif
  }
}

//Define interrupt for motion sensors
void IRAM_ATTR motionISR() {
  
}

void queueHandlerTask(void* pvParamaters) {
  def_message_struct msg;
  for (;;) {
    if (SU2.queue->receive(msg)) {
      def_message_struct response;
      handleRequestSU(msg, &response);
    }
  }
}

esp_now_peer_info_t peerInfo;

void setup() {
  // put your setup code here, to run once:
  sens_unit_ptr = &SU2;
  Serial.begin(115200);
  motion.add(MOTION_SENSOR_PIN);
  lcd.begin(SDA_PIN, SCL_PIN);
  lcd.backlight();
  lcd.print("Temp");
  SU2.modules = module;
  SU2.motion = &motion;
  SU2.moduleCount = 1;
  SU2.queue = q;
  
  SU2.dht_sensor = nullptr;
  SU2.gpsSerial = nullptr;
  SU2.gps = nullptr;

  if (!initSU(&SU2)) {
    strncpy(SU2.name, "MOTION SENSOR 1", sizeof(SU2.name));
  }
  memcpy(peerInfo.peer_addr, communication_unit_addr, 6);
  memcpy(SU2.CU_ADDR, communication_unit_addr, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer exiting");
    exit(-1);
  }
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));
  
}

void loop() {
  
}
