#include <DHT.h>
#include <esp_now.h>
#include <WiFi.h>
#include <string.h>
#include <time.h>
#include "src/sensor_units_lib/sensor_units.h"

#define PPS_PIN 34
#define DHTTYPE DHT11
#define DHTPIN 4
DHT dht(DHTPIN, DHTTYPE);

//Board address here 
uint8_t broadcastAddress[] = {0x3C, 0x8A, 0x1F, 0xD3, 0xD6, 0xEC};
const char* module = "temp_and_humid_SU";
esp_now_peer_info_t peerInfo;
def_message_struct recieve;

sensor_unit SU1;
int num_of_sensors = 2;
enum sensor_type SU1_MODULES[] = {TEMP_AND_HUMID, GPS};

void setup() {
  dht.begin();
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing espNOW");
    exit(0);
  }

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = true;

  if (esp_now_add_peer(&peerInfo)!= ESP_OK) {
    Serial.println("Failed to add peer");
    exit(0);
  }
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(onDataRecv));
}


void loop() {

}


void onDataSent(const u_int8_t *addr, esp_now_send_status_t status) {
  Serial.println("Packet sent");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery success":"Delivery failed");
}

void onDataRecv(const u_int8_t * adr, const u_int8_t * data, int len) {
  memset(&recieve, 0, sizeof(recieve));
  memcpy(&recieve, data, sizeof(recieve));

}





