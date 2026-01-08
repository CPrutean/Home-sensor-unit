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

  // Iterate through all sensor units managed by the manager
  for (int i = 0; i < MAXPEERS; i++) {
    if (manager->sensorCount[i] == 0) continue; // Skip if no sensors for this unit

    JsonObject sensorUnitObj = sensorUnits.add<JsonObject>();
    sensorUnitObj["unitIndex"] = i;
    sensorUnitObj["unitId"] = "SensorUnit-" + String(i);

    // Add status information
    const char* statusStr = "UNKNOWN";
    switch(manager->suStatus[i]) {
      case SensorUnitManager::ONLINE: statusStr = "ONLINE"; break;
      case SensorUnitManager::ERROR: statusStr = "ERROR"; break;
      case SensorUnitManager::OFFLINE: statusStr = "OFFLINE"; break;
      default: statusStr = "UNKNOWN"; break;
    }
    sensorUnitObj["status"] = statusStr;

    // Create sensors array for this sensor unit
    JsonArray sensors = sensorUnitObj["sensors"].to<JsonArray>();

    // For each sensor definition in this sensor unit
    for (int j = 0; j < manager->sensorCount[i]; j++) {
      SensorDefinition &sensorDef = manager->sensors[i][j];

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
      SensorUnitReadings &suReadings = manager->readingsArr[i];
      int numReadings = suReadings.getReadingCount();

      // Add available readings
      for (int k = 0; k < sensorDef.numValues && k < 2; k++) {
        JsonObject reading = readings.add<JsonObject>();
        reading["name"] = String(sensorDef.readingStringsArray[k]);
        reading["value"] = 0; // Placeholder - actual value extraction needed
        reading["timestamp"] = millis() / 1000;
      }
    }
  }

  String response;
  serializeJson(doc, response);

  server.send(200, "application/json", response);
}
