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
  uint8_t numValues{};
  Sensors_t sensor{Sensors_t::NUM_OF_SENSORS};
  char name[30]{'\0'};
  char readingStringsArray[2][12]{{'\0'}};
  Packet::PacketType_T msgType[2]{Packet::NUMTYPES}; //Should only ever be reading or post, other methods will throw errors if this isnt true
  void *fnParam{nullptr}; //Functions should have a param for the packet and the index as well as a reference to the sensorUnit
  //Serializes the sensor unit for transmission over a string
  void toString(char *buffer, size_t sizeOfBuffer) {
    if (sizeOfBuffer == 0)
      return;

    buffer[0] = '\0';
    size_t currentLen = 0;
    int written = 0;

    written = snprintf(buffer, sizeOfBuffer, "%s%s", name, STRSEPER);
    if (written < 0 || (size_t)written >= sizeOfBuffer)
      return;
    currentLen += written;

    for (size_t i = 0; i < numValues; i++) {
      size_t remainingSpace = sizeOfBuffer - currentLen;
      written = snprintf(buffer + currentLen, remainingSpace, "%s%s", readingStringsArray[i], STRSEPER);
      if (written < 0 || (size_t)written >= remainingSpace) break;
      currentLen += written;
    }
  }

  //Deserializes a sensor definition
  void fromString(char *buffer, size_t size) {
    numValues = 0;
    memset(readingStringsArray, 0, sizeof(readingStringsArray));

    int ind1 = 0;
    int wordCount = 0;

    for (size_t i = 0; i < size; i++) {
      bool isSeparator = (buffer[i] == STRSEPER[0]);
      bool isEnd = (buffer[i] == '\0');

      if (isSeparator || isEnd) {
        int len = i - ind1;

        if (wordCount == 0) {
          snprintf(name, sizeof(name), "%.*s", len, buffer + ind1);
        } else if (wordCount - 1 < 5) {
          snprintf(readingStringsArray[wordCount - 1], sizeof(readingStringsArray[0]), "%.*s", len, buffer + ind1);
        }

        ind1 = i + 1;
        wordCount++;

        if (isEnd)
          break;
      }
    }
    fnParam = nullptr;
    if (wordCount > 0)
      numValues = wordCount - 1;
  }

  //Assignment for NULL to allow easier array iteration
  void operator = (long L) {
    if (ESP_ERROR_CHECK_WITHOUT_ABORT(L == NULL && "ONLY USED FOR ASSIGNING NULL VALUES") != ESP_OK) {
      Serial.println("Invalid value passed");
      return;
    }

    numValues = L;
    sensor = Sensors_t::NUM_OF_SENSORS;
  }


  //Comparison operator for NULL to allow while loop iterations
  bool operator == (long L) {
    return numValues == L && sensor == Sensors_t::NUM_OF_SENSORS;    
  }


};



void initSensorDefinition(SensorDefinition &sensorDef);


//Union to convert data iteration
union dataConverter {
  uint8_t data[MAXPACKETSIZE]{};
  int i;
  float f;
  double d;
  char str[MAXPACKETSIZE];
};

//For message acknowledgement systems
struct ackListItem {
  unsigned long long msgID{0};
  unsigned long long postTime{};
  Packet packetList[8]{};
  size_t packetCount{0};
};
