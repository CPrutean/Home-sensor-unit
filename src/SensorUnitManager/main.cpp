#include "../Local_config.h"
#include "WiFiType.h"
#include "freertos/idf_additions.h"
#include "portmacro.h"
#include <Core/global_include.h>
#include <SensorUnitManager/SensorUnitManager.h>
#include <WebComponents/DashboardAPI.h>

SensorUnitManager SUM(CONFIG::SUMAC, CONFIG::PMKKEY, CONFIG::SULMKKEYS);


void printPacket(const Packet &p) {
  String str = "Packet recieved of type  ";
  switch (p.type) {
  case (Packet::ACK):
    str += "ACK ";
    break;
  case (Packet::READING):
    str += "READING ";
    break;
  case (Packet::POST):
    str += "POST";
    break;
  case (Packet::PING):
    str += "PING";
    break;
  case (Packet::FIN):
    str += "FIN";
    break;
  default:
    str += "???";
    break;
  };

  str += " of data type and value ";
  switch (p.dataType) {
  case (Packet::STRING_T):
    str += "STRING ";
    str += p.str;
    break;
  case (Packet::INT_T):
    str += "INT ";
    str += String(p.i);
    break;
  case (Packet::FLOAT_T):
    str += "FLOAT ";
    str += String(p.f);
    break;
  case (Packet::DOUBLE_T):
    str += "DOUBLE ";
    str += String(p.d);
    break;
  case (Packet::NULL_T):
    str += "NULL";
  default:
    str += "???";
    break;
  };

  str += "With sensor ";
  switch (p.info.sensor) {
  case (Sensors_t::BASE):
    str += "BASE";
    break;
  case (Sensors_t::MOTION):
    str += "MOTION";
    break;
  case (Sensors_t::TEMPERATURE_AND_HUMIDITY):
    str += "TEMPERATURE AND HUMIDITY";
    break;
  default:
    "???";
    break;
  };

  str += "and ind ";
  str += String(p.info.ind);

  Serial.println(str);
}

static Packet p;
void packetHandlerTask(void *parameters) {
  for (;;) {
    if (SUM.msgQueue.receive(p)) {
      // printPacket(p);
      SUM.handlePacket(p);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// Responsible for pinging the sensor units every few seconds
void pingTask(void *parameters) {
  for (;;) {
    SUM.pingAllSU();
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  SUM.initESPNOW();

  // Assuming esp-now init code works fine then this should work just fine
  for (uint8_t i{0}; i < 2; i++) {
    SUM.addNewSU(CONFIG::SUMAC[i]);
  }

  for (int i{0}; i < SUM.getSuCount(); i++) {
    SUM.initSensorUnitSensors(i);
  }

  bool connected{false};

  xTaskCreatePinnedToCore(packetHandlerTask, "Packet Handler Task", 8096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(pingTask, "Sensor Ping Task", 2048, NULL, 1, NULL, 1);
}

void loop() {}
