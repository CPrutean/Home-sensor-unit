#include <Arduino.h>
#include "SensorUnits/SensorUnits.h"
#include "../Local_config.h"

#define DHTTYPE DHT22
#define DHTPIN 13

DHT dht(DHTPIN, DHTTYPE);
SensorUnit SU2(CONFIG::CUMAC, CONFIG::PMKKEY, CONFIG::SUMLMKKEY, &dht);
TaskHandle_t PacketTaskHandle;

void packetHandlerTask(void* params) {
    Packet p;
    for (;;) {
        if (SU2.msgQueue.receive(p)) {
            SU2.handlePacket(p);
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

void setup() {
    dht.begin();
    Serial.begin(115200);
    SU2.initESPNOW();
    xTaskCreatePinnedToCore(packetHandlerTask, "Packet Handler Task", 8192, NULL, 1, &PacketTaskHandle, 1);
}

void loop() {

}