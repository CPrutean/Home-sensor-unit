#include <sensor_units.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


#define DHTTYPE DHT22
#define DHTPIN 4

#define PPS_PIN 34
#define RXPIN 0
#define TXPIN 1 

DHT dht(DHTPIN, DHTTYPE);
HardwareSerial gpsSerial(2);
TinyGPSPlus gps;

uint8_t broadcastAddress[] = {0x08, 0xa6, 0xf7, 0x70, 0x10, 0x04};
const char* module = "temp_and_humid_SU";
esp_now_peer_info_t peerInfo;
//Define both of these in each .ino file
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

void queueHandlerTask(void* pvParamaters) {
  for (;;) {
    if (!SU1.queue->isEmpty()) {
      def_message_struct msg;
      msg = SU1.queue->getFront();

      def_message_struct response;
      memset(&response, 0, sizeof(response));
      response.message[0]  = '\0';

      handleRequestSU(msg.message, &response);
      SU1.queue->pop();
    }
  }
}


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
  int i;
  for (i = 0; i < 6; i++) {
    SU1.CU_ADDR[i] = broadcastAddress[i];
  }

  init_SU_ESPNOW(sens_unit_ptr, 0);

  xTaskCreatePinnedToCore(queueHandlerTask, "Queue handler task", 8192, NULL, 1, NULL, 1);
}

void loop() {
  if (ppsDetected) {
    noInterrupts();
    ppsDetected = false;
    unsigned long currentPpsCount = ppsCount;
    unsigned long currentLastPpsMillis = lastPpsMillis;
    interrupts();
  }
}