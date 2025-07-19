#include <DHT.h>
#include <esp_now.h>
#include <WiFi.h>
#define DHTTYPE DHT11
#define DHTPIN 4
DHT dht(DHTPIN, DHTTYPE);

typedef struct struct_message{
  char* message;
  float value;
  int urgencyLevel;
} struct_message;

//Board address here 
uint8_t broadcastAddress[] = {};


const char* moduleType = "temp_and_humid_SU";

struct_message MSG;

esp_now_peer_info_t peerInfo;

void onDataSent(const u_int8_t *addr, esp_now_send_status_t status) {

}

void setup() {
  // put your setup code here, to run once:
  dht.begin();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing espNOW");
    return;
  }
}

void loop() {

}

float requestTemp() {
  return dht.readTemperature();
}

float requestHumidity() {
  return dht.readHumidity();
}







