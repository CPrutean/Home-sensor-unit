#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <etl/optional.h>
#include <etl/vector.h>
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/task.h>

#define MAXPACKETSIZE 96 
#define MAXREADINGPERSENSOR 10


const char STRSEPER[2] = {'|', '\0'};


/*
@breif: enum for different types of sensors
*/
enum class Sensors_t :uint8_t {
  TEMPERATURE_AND_HUMIDITY = 0,
  MOTION,
  BASE,
  NUM_OF_SENSORS
};

//Packet identifier for communication units
//Will be used to identify the sensors and indexes for POST packet types
struct PacketInfo_t {
  Sensors_t sensor{Sensors_t::NUM_OF_SENSORS};
  uint8_t ind{255};
};

//Union to convert data iteration
union dataConverter {
  uint8_t data[MAXPACKETSIZE]{0};
  int i;
  float f;
  double d;
  char str[MAXPACKETSIZE];
};


//Packets are sent between sensorUnits and used to communicate with the devices
struct Packet {
  uint8_t packetData[MAXPACKETSIZE]{};
  unsigned long long msgID{};
  PacketInfo_t info{};
  uint8_t size{};
  enum PacketType_T : uint8_t { PING = 0, ACK, READING, POST, FIN, NUMTYPES };
  enum DataType_T : uint8_t { DOUBLE_T = 0, STRING_T, FLOAT_T, INT_T, NULL_T };
  PacketType_T type{NUMTYPES};
  DataType_T dataType{NULL_T};

  //Converts from byte array to the dataConverter union to pull the data
  void convert(dataConverter &convert) {
    if (size <= 0) {
      return;
    }
    memcpy(convert.data, packetData, size);
  }

  //Takes a data value and converts into a packet
  void writeToPacket(const dataConverter& d, size_t sizeIn) {
    size = sizeIn;
    memcpy(packetData, d.data, size);
  }
};



//Defines a sensor and the function which handles the readings and the indexes
//Will be sent to the SensorUnitManager over a string to let the SensorUnitManager 
//know what sensor units are available
struct SensorDefinition {
  char readingStringsArray[2][12]{{'\0'}, {'\0'}};
  char name[20]{'\0'};
  Packet::PacketType_T msgType[2]{Packet::NUMTYPES}; //Should only ever be reading or post, other methods will throw errors if this isnt true
  Packet::DataType_T dataType[2]{Packet::NULL_T};
  Sensors_t sensor{Sensors_t::NUM_OF_SENSORS};
  uint8_t numValues{};
  void* fnMemAdr{nullptr};


  void toString(char *buffer, size_t sizeOfBuffer) {
    if (sizeOfBuffer == 0 || buffer == nullptr) {
      Serial.println("FAILED TO SERIALIZE: BUFFER INVALID");
      return;
    }

    size_t offset = 0;
    int written = 0;

    written = snprintf(buffer + offset, sizeOfBuffer - offset, "%s%s", name, STRSEPER);
    
    if (written < 0 || (size_t)written >= sizeOfBuffer - offset) return; 
    offset += written;

    for (int i = 0; i < numValues; i++) {
      written = snprintf(buffer + offset, sizeOfBuffer - offset, "%s%s", readingStringsArray[i], STRSEPER);
      if (written < 0 || (size_t)written >= sizeOfBuffer - offset) break;
      offset += written;

      written = snprintf(buffer + offset, sizeOfBuffer - offset, "%d%s", static_cast<int>(msgType[i]), STRSEPER);
      if (written < 0 || (size_t)written >= sizeOfBuffer - offset) break;
      offset += written;
    }
  }

  //Deserializes a sensor definition
  void fromString(char *buffer, size_t size) {
  if (size == 0 || buffer == nullptr) {
    Serial.println("FAILED TO DESERIALIZE: BUFFER INVALID");
    return;
  }

  int ind1 = 0;
  int tokenCount = 0;
  int arrayIndex = 0;
  numValues = 0; 

  for (int i = 0; i < size; i++) {
    if (buffer[i] == STRSEPER[0] || buffer[i] == '\0') {
      int tokenLen = i - ind1;

      if (tokenCount == 0) {
        snprintf(name, sizeof(name), "%.*s", tokenLen, buffer + ind1);
      } 
      else if (tokenCount % 2 != 0) {
        arrayIndex = (tokenCount - 1) / 2;
        if (arrayIndex < 10) { 
          snprintf(readingStringsArray[arrayIndex], sizeof(readingStringsArray[arrayIndex]), "%.*s", tokenLen, buffer + ind1);
        }
      } 
      else {
        char digitChar = buffer[ind1];
        if (digitChar >= '0' && digitChar <= '9') {
          msgType[arrayIndex] = static_cast<Packet::PacketType_T>(digitChar-'0');
        } else {
          msgType[arrayIndex] = Packet::NUMTYPES; 
        }
        numValues++;
      }

      tokenCount++;
      ind1 = i + 1; 
    }
  } 
  }
};



void initSensorDefinition(SensorDefinition &sensorDef);


//For message acknowledgement systems
struct ackListItem {
  unsigned long long msgID{0};
  unsigned long long postTime{};
  Packet packetList[8]{};
  size_t packetCount{0};
};
