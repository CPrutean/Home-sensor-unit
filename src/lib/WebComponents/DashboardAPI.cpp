#include "DashboardAPI.h"
#include "dashboard.h"
#include <WiFi.h>
#include <ArduinoJson.h>

// Dashboard API for SensorUnitManager - aggregates data from all sensor units
void initDashboardAPI(WebServer &server, SensorUnitManager *manager) {
  // Serve the dashboard HTML page
  server.on("/", HTTP_GET, [&server]() {
    server.send(200, "text/html", DASHBOARD_HTML);
  });

  server.on("/dashboard", HTTP_GET, [&server]() {
    server.send(200, "text/html", DASHBOARD_HTML);
  });

  // API endpoint for aggregated sensor data from all sensor units
  server.on("/api/sensors", HTTP_GET, [&server, manager]() {
    handleSensorDataAPI(server, manager);
  });
}

static const char hexaDecimalArr[] = {"0123456789abcdef"};
static void macToString(String &str, uint8_t *mac) {
  if (str != "") { //String should be empty
    return;
  }
  for (int i{0}; i < 6; i++) {
    str+=hexaDecimalArr[mac[i]/16];
    str+=hexaDecimalArr[mac[i]%16];
  }
}

// Handle API request to get aggregated sensor data from all sensor units
void handleSensorDataAPI(WebServer &server, SensorUnitManager *manager) {
  if (manager == nullptr) {
    server.send(500, "application/json", "{\"error\":\"Sensor unit manager not initialized\"}");
    return;
  }

  JsonDocument doc;

  // Add manager system information
  doc["deviceId"] = WiFi.macAddress();
  doc["deviceType"] = "SensorUnitManager";
  doc["uptime"] = millis() / 1000;
  doc["rssi"] = WiFi.RSSI();
  doc["freeMemory"] = ESP.getFreeHeap();

  // Create sensor units array - group sensors by their sensor unit
  JsonArray sensorUnits = doc["sensorUnits"].to<JsonArray>();
  SensorUnitManager::SensorUnitInfo info;
  // Iterate through all sensor units managed by the manager
  for (int i = 0; i < manager->getSuCount(); i++) {
    if (info.sensorCount == 0) continue; // Skip if no sensors for this unit

    info = manager->getSensorUnitInfo(i);

    JsonObject sensorUnitObj = sensorUnits.add<JsonObject>();
    sensorUnitObj["unitIndex"] = i;
    sensorUnitObj["unitId"] = "SensorUnit-" + String(i);

    // Add status information
    const char* statusStr = "UNKNOWN";
    switch(info.status) {
      case SensorUnitManager::ONLINE: statusStr = "ONLINE"; break;
      case SensorUnitManager::ERROR: statusStr = "ERROR"; break;
      case SensorUnitManager::OFFLINE: statusStr = "OFFLINE"; break;
      default: statusStr = "UNKNOWN"; break;
    }
    sensorUnitObj["status"] = statusStr;

    // Create sensors array for this sensor unit
    JsonArray sensors = sensorUnitObj["sensors"].to<JsonArray>();

    // For each sensor definition in this sensor unit
    for (int j = 0; j < info.sensorCount; j++) {
      SensorDefinition &sensorDef = info.sensors[j];

      JsonObject sensorObj = sensors.add<JsonObject>();
      sensorObj["name"] = String(sensorDef.name);
      sensorObj["sensorIndex"] = j;

      // Map sensor type
      if (sensorDef.sensor == Sensors_t::TEMPERATURE_AND_HUMIDITY) {
        sensorObj["type"] = "TEMPERATURE_AND_HUMIDITY";
      } else if (sensorDef.sensor == Sensors_t::MOTION) {
        sensorObj["type"] = "MOTION";
      } else {
        sensorObj["type"] = "BASE";
      }

      // Add readings array
      JsonArray readings = sensorObj["readings"].to<JsonArray>();

      // Get readings from the SensorUnitReadings array
      SensorUnitReadings &suReadings = info.readings;
      int numReadings = suReadings.getReadingCount();

      // Add available readings
      dataConverter d;
      for (uint8_t k = 0; k < sensorDef.numValues && k < 2; k++) {
        JsonObject reading = readings.add<JsonObject>();
        reading["name"] = String(sensorDef.readingStringsArray[k]);
        String readingVal{};
        suReadings.getReading(d.data, sizeof(d), {sensorDef.sensor, k});
        switch(sensorDef.dataType[k]) {
          case(Packet::STRING_T): readingVal = d.str; break;
          case(Packet::DOUBLE_T): readingVal = String(d.d); break;
          case(Packet::FLOAT_T): readingVal = String(d.f); break;
          case(Packet::INT_T): readingVal = String(d.i); break;
          case(Packet::NULL_T): readingVal = "NULL"; break;
          default: readingVal = "INVALID TYPE"; break;
        };
        reading["value"] = readingVal; 
        reading["timestamp"] = millis() / 1000;
      }
    }
  }

  String response;
  ArduinoJson::serializeJson(doc, response);

  server.send(200, "application/json", response);
}
