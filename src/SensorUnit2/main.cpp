#include <Arduino.h>
#include <DHT.h>
#define DHTTYPE DHT22
#define DHTPIN 13

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    dht.begin();
    Serial.begin(115200);
    Serial.println("Setup");

}

void loop() {
    float val1{dht.readTemperature()};
    float val2{dht.readHumidity()};
    Serial.print("HUMID: ");
    Serial.println(val2);
    Serial.print("TEMP: ");
    Serial.println(val1);
    delay(1000);
}