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

void setup() {
  // put your setup code here, to run once:
  dht.begin();
}

void loop() {

}

float requestTemp() {
  return dht.readTemperature();
}

float requestHumidity() {
  return dht.readHumidity();
}







