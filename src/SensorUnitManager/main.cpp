#include "../Local_config.h"
#include <Core/global_include.h>
#include <SensorUnitManager/SensorUnitManager.h>
WebServer server(80); // Web server on port 80
SensorUnitManager SUM(CONFIG::SUMAC, 2, server, CONFIG::PMKKEY,
                      CONFIG::SULMKKEYS);

//Soley for debugging
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

//Handles packets
void packetHandlerTask(void *parameters) {
  Packet p;
  for (;;) {
    if (SUM.msgQueue.receive(p)) {
      SUM.handlePacket(p);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// Responsible for pinging the sensor units every few seconds
void pingTask(void *parameters) {
  Packet p;
  for (;;) {
    for (int i{0}; i < SUM.getSuCount(); i++) {
    }
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

// Will contain an int buffer, which will only change the internal error state
// and update status
void errorCheckerTask(void *parameters) {
  SensorUnitStatus
      status[6]{}; // Online is implicitly 0 so this initializes it to online
  int i{0};
  for (;;) {
    SUM.msgAck.removeTimedOutReq(status);
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

void setup() {
<<<<<<< HEAD
    Serial.begin(115200);
    SUM.initESPNOW();
    //Assuming esp-now init code works fine then this should work just fine
    for (int i{0}; i < SUM.getSuCount(); i++) {
        SUM.initSensorUnitSensors(i);
    }
    xTaskCreatePinnedToCore(packetHandlerTask, "Packet Handler Task", 8192, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(pingTask, "Sensor Ping Task", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(errorCheckerTask, "Packet Status Checker", 2048, NULL, 1, NULL, 0);
=======
  Serial.begin(115200);
  SUM.initESPNOW();
  // Assuming esp-now init code works fine then this should work just fine
  for (int i{0}; i < SUM.getSuCount(); i++) {
    SUM.initSensorUnitSensors(i);
  }
>>>>>>> 6cc64dfebdbed8485514fe5c3a61a680deec0b31
}

void loop() {}
