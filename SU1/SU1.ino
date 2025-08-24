#include <sensor_units.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>


#define DHTTYPE DHT22
#define DHTPIN 4

#define PPS_PIN 34
#define RXPIN 16
#define TXPIN 17
//Define this when you are monitoring serial output
//#define DEBUG 1


DHT dht(DHTPIN, DHTTYPE);
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;

const uint8_t broadcastAddress[] = {0x08, 0xa6, 0xf7, 0x70, 0x10, 0x04};
const char* module = "temp_and_humid_SU";

sensor_unit *sens_unit_ptr;
communication_unit *com_unit_ptr;
sensor_unit SU1;

sensor_type SU1_MODULES[] = {TEMP_AND_HUMID, GPS};

msg_queue message_queue;

volatile bool ppsDetected = false;
volatile unsigned long ppsCount = 0;
volatile unsigned long lastPpsMillis = 0;

void ppsISR() {
  ppsCount++;
  lastPpsMillis = millis();
  ppsDetected = true;
}


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
  if (SU1.queue->send(msg)) {
    #ifdef DEBUG
    Serial.println("Message succesfully sent to queue");
    #endif
  } else {
    #ifdef DEBUG
    Serial.println("The message failed to be added to queue");
    #endif
  }
}

void sensorReaderTask(void* pvParameters) {
  for (;;) {
    if (ppsDetected) {
      noInterrupts();
      ppsDetected = false;
      interrupts();
      
      while (SU1.gpsSerial->available() > 0) {
        SU1.gps->encode(SU1.gpsSerial->read());
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20)); 
  }
}


void queueHandlerTask(void* pvParamaters) {
  def_message_struct msg;
  for (;;) {
    if (SU1.queue->receive(msg)) {
      def_message_struct response;
      handleRequestSU(msg.message, &response);
    }
  }
}


esp_now_peer_info_t peerInfo;

void setup() {
  dht.begin();
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, RXPIN, TXPIN);

  pinMode(PPS_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PPS_PIN), ppsISR, RISING);
  //These need to be declared across all SU and CU libraries
  sens_unit_ptr = &SU1;
  com_unit_ptr = nullptr;

  SU1.modules = SU1_MODULES;
  SU1.moduleCount = 2;
  SU1.gpsSerial = &gpsSerial;
  SU1.dht_sensor = &dht;
  SU1.gps = &gps;
  SU1.queue = &message_queue;
  SU1.name[0] = '\0';

  def_message_struct temp;
  if (!initSU(&SU1)) {
    snprintf(SU1.name, MAX_NAME_LEN, "%s", "Temp and humidity");    
  }

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    #ifdef DEBUG
    Serial.println("Failed to init exiting");
    #endif
    exit(-1);
  }
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  memcpy(SU1.CU_ADDR, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    #ifdef DEBUG
    Serial.println("Failed to add peer exiting");
    #endif
    exit(-1);
  }
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));

  xTaskCreatePinnedToCore(queueHandlerTask, "Queue handler task", 8192, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(sensorReaderTask, "Sensor reading task", 4096, NULL, 1, NULL, 1);
}

void loop() {
  
}