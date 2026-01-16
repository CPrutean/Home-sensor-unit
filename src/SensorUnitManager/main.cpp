#include "../Local_config.h"
#include <Core/global_include.h>
#include <SensorUnitManager/SensorUnitManager.h>
WebServer server(80); // Web server on port 80
SensorUnitManager SUM(CONFIG::SUMAC, 2, server, CONFIG::PMKKEY,
                      CONFIG::SULMKKEYS);

// Will handle all packets within the queue
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
  Serial.begin(115200);
  SUM.initESPNOW();
  // Assuming esp-now init code works fine then this should work just fine
  for (int i{0}; i < SUM.getSuCount(); i++) {
    SUM.initSensorUnitSensors(i);
  }
}

void loop() {}
