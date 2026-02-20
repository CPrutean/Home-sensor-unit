#include <Arduino.h>
#include <PIR.h>
#include <SensorUnits/SensorUnits.h>
#include "../Local_config.h"
#include <WebServer-SUM-portal/Portal.h>

#define PIRPIN 13
PIR pir;
SensorUnit SU1(CONFIG::CUMAC, CONFIG::PMKKEY, CONFIG::SUMLMKKEY, nullptr, &pir);

void packetHandlerTask(void* params) {
    Packet p;
    for (;;) {
        if (SU1.msgQueue.receive(p)) {
            SU1.handlePacket(p);
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void setup() {
    Serial.begin(115200);
    pir.add(PIRPIN);
    pinMode(PIRPIN, INPUT);
    SU1.initESPNOW();
    xTaskCreatePinnedToCore(packetHandlerTask, "Packet Handler Task", 8192, NULL, 1, NULL, 1);
}

void loop() {

}
