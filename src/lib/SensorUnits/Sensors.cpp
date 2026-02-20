#include "SensorUnits.h"

/**
@breif: initializes the internal SensorDefinition as well as the internal
function pointer to handle commands requested
@param: SensorDefinition &sensorDef: SensorDefinition to be assigned
*/
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
    sensorDef.msgType[0] = Packet::READING;
    snprintf(sensorDef.readingStringsArray[1],
             sizeof(sensorDef.readingStringsArray[1]), "%s", "HUMIDITY");
    sensorDef.msgType[1] = Packet::READING;

    sensorDef.fnMemAdr = reinterpret_cast<void *>(tempCommands);
    sensorDef.numValues = 2;

    break;
  case (Sensors_t::MOTION):
    snprintf(sensorDef.name, sizeof(sensorDef.name), "%s", "MOTION_SENSOR");

    snprintf(sensorDef.readingStringsArray[0],
             sizeof(sensorDef.readingStringsArray[0]), "%s", "MOTION");
    sensorDef.msgType[0] = Packet::READING;

    sensorDef.fnMemAdr = reinterpret_cast<void *>(motionCommands);
    sensorDef.numValues = 1;
    break;
  case (Sensors_t::BASE):
    snprintf(sensorDef.name, sizeof(sensorDef.name), "%s", "BASE");

    snprintf(sensorDef.readingStringsArray[0],
             sizeof(sensorDef.readingStringsArray[0]), "%s", "INIT");
    sensorDef.msgType[0] = Packet::READING;

    snprintf(sensorDef.readingStringsArray[1], sizeof(sensorDef.name), "%s",
             "PING");
    sensorDef.msgType[1] = Packet::READING;

    snprintf(sensorDef.readingStringsArray[2],
             sizeof(sensorDef.readingStringsArray[2]), "%s",
             "ERROR"); // Sent to sensorUnitManagers when something erroneous
                       // has happened
    sensorDef.msgType[2] = Packet::READING;
    // Should always be sent with an error code
    sensorDef.fnMemAdr = reinterpret_cast<void *>(baseCommands);
    sensorDef.numValues = 1;
    break;
  default:
    Serial.println("Failed to init");
    break;
  };
}

/**
 * @breif: Method that defines temperature sensor functionality,
 * will return a packet with error message if faulty reading is recieved
 * @param: Sensor unit object reference for the command to work
 * @param: Packet we are writing for sensor unit to send
 * @param: index of the command we are calling
 */
void tempCommands(SensorUnit &sensUnit, Packet &p, uint8_t ind) {
  if (sensUnit.temp == nullptr) {
    writeErrorMsg(p, "INVALID SENSOR POINTER");
    return;
  }
  p.type = Packet::READING;
  p.info.sensor = Sensors_t::TEMPERATURE_AND_HUMIDITY;

  if (ind == 0) {
    p.f = sensUnit.temp->readTemperature();
    p.dataType = Packet::FLOAT_T;
    p.info.ind = ind;
  } else if (ind == 1) {
    p.f = sensUnit.temp->readHumidity();
    p.dataType = Packet::FLOAT_T;
    p.info.ind = ind;
  } else {
    writeErrorMsg(p, "INVALID INDEX PASSED");
  }
}
/**
 * @breif: Method that defines motion sensor functionality.
 * Error messages will be written to the packet if faulty state is found
 * @param: Sensor unit reference we are pulling the pointer from
 * @param: Packet we are sending to the sensor unit
 * @param: Index of the command we are pulling
 */
void motionCommands(SensorUnit &sensUnit, Packet &p, uint8_t ind) {
  p.type = Packet::READING;
  p.info.sensor = Sensors_t::MOTION;
  if (sensUnit.motion == nullptr) {
    writeErrorMsg(p, "SENSOR POINTER WAS NEVER INITIALIZED");
    return;
  }

  if (ind == 0) {
    p.i = sensUnit.motion->read();
  } else {
    writeErrorMsg(p, "INVALID INDEX PASSED");
  }
}
/**
 * @breif: Method that defines commands available to every sensor unit
 * @param: Sensor unit we are pulling information from
 * @param: Packet we are writing to
 * @param: Index of the default command we are requesting
 */

// INIT, PING, ERROR
void baseCommands(SensorUnit &sensUnit, Packet &p, uint8_t ind) {
  if (ind == 0) { // INIT
    int i{0};
    for (i = 0; i < sensUnit.sensCount; i++) {
      p.str[0] = '\0';
      p.info.ind = ind;
      p.type = Packet::READING;
      p.dataType = Packet::STRING_T;
      p.info.sensor = Sensors_t::BASE;
      sensUnit.sensorsAvlbl[i].toString(p.str, sizeof(p.str));
      sensUnit.sendPacket(p);
    }
  } else if (ind == 1) { // PING
    sendAllPackets(sensUnit);
  } else { // ERROR
    // Assume erronious ind otherwise, however sending a packet with BASE ind =
    // 2 tells SensorUnitmanager there was an error
    writeErrorMsg(p, "INVALID IND");
  }
}

/**
 * @breif: Helper method we use to write error messages to packets
 * @param: Packet we are writing to
 * @param: data converter we use to help to write to the packet
 * @param: error message we are writing to the packet
 */
void writeErrorMsg(Packet &p, const char *errormsg) {
  p.info.sensor = Sensors_t::BASE;
  p.info.ind = 2;
  p.dataType = Packet::STRING_T;
  size_t len = strlen(errormsg);
  snprintf(p.str, len, "%s", errormsg);
}
