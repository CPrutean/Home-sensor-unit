#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/FreeRTOSConfig.h>

#define MAXPACKETSIZE 64
#define MAXREADINGPERSENSOR 10

const char STRSEPER[2] = {'|', '\0'};
enum class Sensors_t { TEMPERATURE_AND_HUMIDITY = 0, MOTION, NUM_OF_SENSORS };
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

struct Packet {
  enum DataType_T { STRING_T, DOUBLE_T, FLOAT_T, INT_T };
  uint8_t packetData[MAXPACKETSIZE]{};
  unsigned long long msgID{};
  size_t size{};
  void convert(dataConverter &convert) {
    if (size <= 0) {
      Serial.print("FAILED TO CONVERT INVALID SIZE: ");
      Serial.println(size);
      return;
    }
    memcpy(convert.data, packetData, size);
  }
};
