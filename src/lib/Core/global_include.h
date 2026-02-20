#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>
#include <freertos/task.h>

#define MAXPACKETSIZE 96
#define MAXREADINGPERSENSOR 10
const char STRSEPER[2] = {'|', '\0'};

/*
@breif: enum for different types of sensors
*/
enum class Sensors_t : uint8_t {
  TEMPERATURE_AND_HUMIDITY = 0,
  MOTION,
  BASE,
  NUM_OF_SENSORS
};

// Packet identifier for communication units
// Will be used to identify the sensors and indexes for POST packet types
struct PacketInfo_t {
  Sensors_t sensor{Sensors_t::NUM_OF_SENSORS};
  uint8_t ind{255};

  bool operator==(const PacketInfo_t &p) const {
    return sensor == p.sensor && ind == p.ind;
  }
};

// Packets are sent between sensorUnits and used to communicate with the devices
struct Packet {
  union {
    char str[MAXPACKETSIZE]{0};
    double d;
    float f;
    int i;
  };
  uint8_t senderAddr[6]{};
  PacketInfo_t info{};
  uint8_t size{};
  enum PacketType_T : uint8_t { PING = 0, ACK, READING, POST, FIN, NUMTYPES };
  enum DataType_T : uint8_t { DOUBLE_T = 0, STRING_T, FLOAT_T, INT_T, NULL_T };
  PacketType_T type{NUMTYPES};
  DataType_T dataType{NULL_T};
};

// Defines a sensor and the function which handles the readings and the indexes
// Will be sent to the SensorUnitManager over a string to let the
// SensorUnitManager know what sensor units are available
struct SensorDefinition {
  char readingStringsArray[2][12]{{'\0'}, {'\0'}};
  char name[20]{'\0'};
  Packet::PacketType_T msgType[2]{
      Packet::NUMTYPES}; // Should only ever be reading or post, other methods
                         // will throw errors if this isnt true
  Packet::DataType_T dataType[2]{Packet::NULL_T};
  Sensors_t sensor{Sensors_t::NUM_OF_SENSORS};
  uint8_t numValues{};
  void *fnMemAdr{nullptr};

  void toString(char *buffer, size_t bufferSize) {
    if (bufferSize == 0 || buffer == nullptr) {
      Serial.println("FAILED TO SERIALIZE: BUFFER INVALID");
      return;
    }

    size_t offset = 0;
    int written = 0;

    written =
        snprintf(buffer + offset, bufferSize - offset, "%s%s", name, STRSEPER);

    if (written < 0 || (size_t)written >= bufferSize - offset)
      return;
    offset += written;

    for (int i = 0; i < numValues; i++) {
      written = snprintf(buffer + offset, bufferSize - offset, "%s%s",
                         readingStringsArray[i], STRSEPER);
      if (written < 0 || (size_t)written >= bufferSize - offset)
        break;
      offset += written;

      written = snprintf(buffer + offset, bufferSize - offset, "%d%s",
                         static_cast<int>(msgType[i]), STRSEPER);
      if (written < 0 || (size_t)written >= bufferSize - offset)
        break;
      offset += written;
    }
  }

  void fromString(const char *buffer, size_t size) {
    if (size == 0 || buffer == nullptr) {
      Serial.println("FAILED TO DESERIALIZE: BUFFER INVALID");
      return;
    }

    int ind1 = 0;
    int tokenCount = 0;
    int arrayIndex = 0;
    numValues = 0;

    // We iterate up to size, but we need to handle the end of the string.
    // We add a check for (i == size) to force processing the last token.
    for (int i = 0; i <= size; i++) {

      // Check for delimiter OR end of buffer.
      // Note: We check (i == size) to catch the end if there is no \0
      bool isEnd = (i == size) || (buffer[i] == '\0');
      bool isDelim = (i < size) && (buffer[i] == STRSEPER[0]);

      if (isEnd || isDelim) {
        int tokenLen = i - ind1;

        // Avoid processing empty tokens if multiple delimiters exist in a row
        if (tokenLen > 0 || tokenCount == 0) {
          if (tokenCount == 0) {
            // Token 0: Name
            snprintf(name, sizeof(name), "%.*s", tokenLen, buffer + ind1);
          } else {
            // Calculate array index based on pairs (Reading + Type)
            // Tokens: 1,2 -> Index 0; 3,4 -> Index 1
            arrayIndex = (tokenCount - 1) / 2;

            // BOUNDS CHECK: Protects BOTH String and Type
            if (arrayIndex < 10) {
              if (tokenCount % 2 != 0) {
                // Odd Tokens: Reading String
                snprintf(readingStringsArray[arrayIndex],
                         sizeof(readingStringsArray[arrayIndex]), "%.*s",
                         tokenLen, buffer + ind1);
              } else {
                // Even Tokens: Msg Type (Integer)
                // Parse integer properly using atoi or strtol logic
                // We create a temporary buffer to null-terminate the number for
                // atoi
                char numBuf[16];
                int safeLen = (tokenLen < 15) ? tokenLen : 15;
                memcpy(numBuf, buffer + ind1, safeLen);
                numBuf[safeLen] = '\0';

                int typeVal = atoi(numBuf);
                msgType[arrayIndex] =
                    static_cast<Packet::PacketType_T>(typeVal);

                numValues++;
              }
            } else {
              // Optional: Log that max sensors exceeded
            }
          }
          tokenCount++;
        }
        ind1 = i + 1;
      }

      // Stop if we hit null terminator naturally to avoid reading past valid
      // data
      if (isEnd && i < size)
        break;
    }
  }
};

void initSensorDefinition(SensorDefinition &sensorDef);

// For message acknowledgement systems
struct ackListItem {
  unsigned long long msgID{0};
  unsigned long long postTime{};
  uint8_t suInd{255};
};
