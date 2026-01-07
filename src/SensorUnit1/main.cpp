#include <Arduino.h>
#include <PIR.h>

#define PIRPIN 13
PIR pir;
void setup() {
    Serial.begin(115200);
    pir.add(PIRPIN);
    pinMode(PIRPIN, INPUT);
    Serial.println("Calibrating sensor please wait");
    delay(60000);
}

void loop() {
    if (digitalRead(PIRPIN) == HIGH) {
        Serial.println("Motion detected");
        delay (500);
    }
}