#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <etl/optional.h>
#include <etl/vector.h>
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/task.h>

#define MAXPACKETSIZE 72
#define MAXREADINGPERSENSOR 10

const char STRSEPER[2] = {'|', '\0'};
enum class Sensors_t :uint8_t {
  TEMPERATURE_AND_HUMIDITY = 0,
  MOTION,
  BASE,
  NUM_OF_SENSORS
};

enum class SensorUnitStatus { ONLINE = 0, ERROR, OFFLINE, NUM_TYPES };

struct SensorDefinition {
  uint8_t numValues{};
  Sensors_t sensor{Sensors_t::NUM_OF_SENSORS};
  char name[30]{'\0'};
  char readingStringsArray[3][11]{{'\0'}};
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
    if (wordCount > 0)
      numValues = wordCount - 1;
  }
  void operator = (long L) {
    if (ESP_ERROR_CHECK_WITHOUT_ABORT(L == NULL && "ONLY USED FOR ASSIGNING NULL VALUES") != ESP_OK) {
      Serial.println("Invalid value passed");
      return;
    }

    numValues = L;
    sensor = Sensors_t::NUM_OF_SENSORS;
  }

  bool operator == (long L) {
    return numValues == L && sensor == Sensors_t::NUM_OF_SENSORS;    
  }
};

void initSensorDefinition(SensorDefinition &sensorDef);

union dataConverter {
  uint8_t data[MAXPACKETSIZE]{};
  int i;
  float f;
  double d;
  char str[MAXPACKETSIZE];
};

struct PacketInfo_t {
  Sensors_t sensor{Sensors_t::NUM_OF_SENSORS};
  uint8_t ind{255};
};

struct Packet {
  uint8_t packetData[MAXPACKETSIZE]{};
  unsigned long long msgID{};
  PacketInfo_t info{};
  uint8_t size{};
  enum PacketType_T : uint8_t { PING = 0, ACK, READING, FIN, NUMTYPES };
  enum DataType_T : uint8_t { DOUBLE_T, STRING_T, FLOAT_T, INT_T, NULL_T };
  PacketType_T type{NUMTYPES};
  DataType_T dataType{NULL_T};
  void convert(dataConverter &convert) {
    if (size <= 0) {
      return;
    }
    memcpy(convert.data, packetData, size);
  }

  void writeToPacket(const dataConverter& d, size_t sizeIn) {
    size = sizeIn;
    memcpy(packetData, d.data, size);
  }
};

struct ackListItem {
  unsigned long long msgID{0};
  unsigned long long postTime{};
  Packet packetList[8]{};
  size_t packetCount{0};
};
