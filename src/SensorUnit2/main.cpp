#include <Arduino.h>
#include "SensorUnits/SensorUnits.h"
#include "../Local_config.h"

#define DHTTYPE DHT22
#define DHTPIN 13

DHT dht(DHTPIN, DHTTYPE);
SensorUnit SU2(CONFIG::CUMAC, CONFIG::PMKKEY, CONFIG::SUMLMKKEY, &dht);
TaskHandle_t PacketTaskHandle;

void printPacket(const Packet& p) {
    String str = "Packet recieved of type  ";
    switch(p.type) {
        case(Packet::ACK): str += "ACK "; break;
        case(Packet::READING): str += "READING "; break;
        case(Packet::POST): str += "POST"; break;
        case(Packet::PING): str += "PING"; break;
        case(Packet::FIN): str += "FIN"; break;
        default: str += "???"; break;
    };
    dataConverter d;

    memcpy(d.data, p.packetData, sizeof(Packet));

    str += " of data type and value ";
    switch(p.dataType) {
        case(Packet::STRING_T): 
            str += "STRING ";
            str += d.str;
            break;  
        case(Packet::INT_T):
            str += "INT ";
            str += String(d.i);
            break;
        case(Packet::FLOAT_T):
            str += "FLOAT ";
            str += String(d.f);
            break;
        case(Packet::DOUBLE_T):
            str += "DOUBLE ";
            str += String(d.d);
            break;
        case(Packet::NULL_T):
            str += "NULL";
        default: str += "???"; break;
    };

    str += "With message id ";
    str += String(p.msgID);
    Serial.println(str);
}

void packetHandlerTask(void* params) {
    Packet p;
    for (;;) {
        if (SU2.msgQueue.receive(p)) {
            SU2.handlePacket(p);
            //printPacket(p);
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