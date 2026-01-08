#include "SensorUnitManager.h"

SensorUnitReadings::SensorUnitReadings() {
    readingCount = 1;
    readings = new uint8_t*[1];
    readings[0] = new uint8_t[4];

    readingSizeArray = new uint8_t;
    readingSizeArray[0] = 4; //Enough to store an int
    readingInfo = new PacketInfo_t[1];
    mutex = xSemaphoreCreateRecursiveMutex();
}

int SensorUnitReadings::getReadingCount() {
    if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("FAILED TO TAKE MUTEX IN SENSOR READINGS RETURNING");
        return -1;
    }
    int num = readingCount;
    xSemaphoreGiveRecursive(mutex);
    return num;
}

void SensorUnitReadings::postReading(const uint8_t *data, uint8_t readingSize, PacketInfo_t packInfo) {
    if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("Failed to take mutex in SensorReadings");
        return;
    }
    bool found = false;
    int i{0};
    for (i = 0; i < readingCount; i++) {
        if (packInfo == readingInfo[i]) {
           found = true;
           break;
        }
    }


    if (!found) {
        readings[readingCount] = new uint8_t[readingSize];
        memcpy(readings[readingCount++], data, readingSize);
        readingSizeArray[readingCount] = readingSize;
        readingInfo[readingCount] = packInfo;
    } else if (readingSize != readingSizeArray[i]) {
        delete[] readings[i];
        readings[i] = new uint8_t[readingSize];
        readingSizeArray[i] = readingSize;
    } else {
        memcpy(readings[i], data, readingSize);
    }

    memcpy(readings[i], data, readingSize);
    xSemaphoreGiveRecursive(mutex);
}

//Will resize internal arrays if larger otherwise will free values that are lower
void SensorUnitReadings::setReadingCount(int num) {
    if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("Failed to take mutex in SensorReadings");
        return;
    }


    if (num == readingCount) {
        xSemaphoreGiveRecursive(mutex);
        return;
    }


    if (num < readingCount) { //Assume a soft reset
        delete[] readingSizeArray;
        delete[] readingInfo;
        for (int i{0}; i < readingCount; i++) {
            delete[] readings[i];
        }
        delete[] readings;

        readingCount = num;
        readings = new uint8_t*[num];
        for (int i{0}; i < num; i++) {
            readings[i] = new uint8_t[4];
        }
        readingInfo = new PacketInfo_t[num];
        readingSizeArray = new uint8_t[num];
    } else {
        uint8_t *newSize = new uint8_t[num];
        PacketInfo_t *newPackInfo = new PacketInfo_t[num];
        uint8_t **newReadings = new uint8_t*[num];
        memcpy(newSize, readingSizeArray, sizeof(readingSizeArray));
        memcpy(newPackInfo, readingInfo, sizeof(readingInfo));
        int i;
        for (i = 0; i < readingCount; i++) {
            newReadings[i] = new uint8_t[readingSizeArray[i]];
            memcpy(newReadings[i], readings[i], readingSizeArray[i]);
        }

        for (int j{i}; j < num; j++) {
            newReadings[j] = new uint8_t[4];
        }

        delete[] readingSizeArray;
        delete[] readingInfo;
        for (int i{0}; i < readingCount; i++) {
            delete[] readings[i];
        }
        delete[] readings;

        readings = newReadings;
        readingSizeArray = newSize;
        readingInfo = newPackInfo;
        readingCount = num;
    }
    xSemaphoreGiveRecursive(mutex);
}


