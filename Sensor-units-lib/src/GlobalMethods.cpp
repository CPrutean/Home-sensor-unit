#include "global_include.h"

void initSensorDefinition(SensorDefinition &sensorDef) {
  if (sensorDef.sensor == Sensors_t::NUM_OF_SENSORS) {
    Serial.println("Invalid object passed initialize a valid sensor");
    return;
  }
  // DEFINE NEW SENSORS HERE
  switch (sensorDef.sensor) {
  case (Sensors_t::TEMPERATURE_AND_HUMIDITY):
    snprintf(sensorDef.name, sizeof(sensorDef.name), "%s",
             "TEMPERATURE_AND_HUMIDITY");
    snprintf(sensorDef.readingStringsArray[0],
             sizeof(sensorDef.readingStringsArray[0]), "%s", "TEMPERATURE");
    snprintf(sensorDef.readingStringsArray[1],
             sizeof(sensorDef.readingStringsArray[1]), "%s", "HUMIDITY");
    sensorDef.numValues = 2;
    break;
  case (Sensors_t::MOTION):
    snprintf(sensorDef.name, sizeof(sensorDef.name), "%s", "MOTION_SENSOR");
    snprintf(sensorDef.readingStringsArray[0],
             sizeof(sensorDef.readingStringsArray[0]), "%s", "MOTION");
    sensorDef.numValues = 1;
    break;
  default:
    Serial.println("Failed to init");
    break;
  };
}
