#include "optional.hpp"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>
namespace std {
template <typename T> using optional = tl::optional<T>;
constexpr auto nullopt = tl::nullopt;
} // namespace std

#define MAXPACKETSIZE 64
#define MAXREADINGPERSENSOR 10

const char STRSEPER[2] = {'|', '\0'};
enum class Sensors_t {
  TEMPERATURE_AND_HUMIDITY = 0,
  MOTION,
  BASE,
  NUM_OF_SENSORS
};

enum class SensorUnitStatus { ONLINE = 0, ERROR, OFFLINE, NUM_TYPES };

struct SensorDefinition {
  Sensors_t sensor{Sensors_t::NUM_OF_SENSORS};
  size_t numValues{};
  char name[30]{'\0'};
  char readingStringsArray[3][10]{{'\0'}};
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
      written = snprintf(buffer + currentLen, remainingSpace, "%s%s",
                         readingStringsArray[i], STRSEPER);
      if (written < 0 || (size_t)written >= remainingSpace)
        break;
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
          snprintf(readingStringsArray[wordCount - 1],
                   sizeof(readingStringsArray[0]), "%.*s", len, buffer + ind1);
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
  enum PacketType_T { PING = 0, ACK, READING, FIN, NUMTYPES };
  enum DataType_T { DOUBLE_T, STRING_T, FLOAT_T, INT_T, NULL_T };
  PacketInfo_t info{};
  uint8_t packetData[MAXPACKETSIZE]{};
  unsigned long long msgID{};
  size_t size{};
  void convert(dataConverter &convert) {
    if (size <= 0) {
      return;
    }
    memcpy(convert.data, packetData, size);
  }
  PacketType_T type{NUMTYPES};
  DataType_T dataType{NULL_T};
};

// When the messageAck recieves the FIN with the corresponding msgID the
// ackListItem returns
struct ackListItem {
  unsigned long long msgID{255};
  Packet packList[8]{};
  size_t packetCount{0};
};
