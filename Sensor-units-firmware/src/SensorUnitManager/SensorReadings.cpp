#include "SensorUnitManager.h"

SensorUnitReadings::SensorUnitReadings() {
    numOfReadings = 1;
    readings = new uint8_t*[1];
    readings[0] = new uint8_t[4];

    sizeOfReadings = new uint8_t;
    sizeOfReadings[0] = 4; //Enough to store an int 
    readingInfo = new PacketInfo_t[1];
    mutex = xSemaphoreCreateRecursiveMutex();
}

int SensorUnitReadings::getNumOfReadings() {
    if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("FAILED TO TAKE MUTEX IN SENSOR READINGS RETURNING");
        return -1;
    }
    int num = numOfReadings;
    xSemaphoreGiveRecursive(mutex);
    return num;
}

void SensorUnitReadings::postReading(const uint8_t *data, uint8_t sizeOfReading, PacketInfo_t packInfo) {
    if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("Failed to take mutex in SensorReadings");
        return;
    }
    bool found = false;
    int i{0};
    for (i = 0; i < numOfReadings; i++) {
        if (packInfo == readingInfo[i]) {
           found = true;
           break; 
        }
    }


    if (!found) {
        Serial.println("Invalid packet info passed");
        xSemaphoreGiveRecursive(mutex);
        return;
    }

    if (sizeOfReading != sizeOfReadings[i]) {
        delete[] readings[i];
        readings[i] = new uint8_t[sizeOfReading];
        sizeOfReadings[i] = sizeOfReading;
    }

    memcpy(readings[i], data, sizeOfReading);
    xSemaphoreGiveRecursive(mutex);
}

//Will resize internal arrays if larger otherwise will free values that are lower
void SensorUnitReadings::setNumOfReadings(int num) {
    if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("Failed to take mutex in SensorReadings");
        return;
    }


    if (num == numOfReadings) {
        xSemaphoreGiveRecursive(mutex);
        return;
    } 


    if (num < numOfReadings) { //Assume a soft reset
        delete[] sizeOfReadings;
        delete[] readingInfo;
        for (int i{0}; i < numOfReadings; i++) {
            delete[] readings[i];
        }
        delete[] readings;

        numOfReadings = num;
        readings = new uint8_t*[num];
        for (int i{0}; i < num; i++) {
            readings[i] = new uint8_t[4];
        }
        readingInfo = new PacketInfo_t[num];
        sizeOfReadings = new uint8_t[num];
    } else {
        uint8_t *newSize = new uint8_t[num];
        PacketInfo_t *newPackInfo = new PacketInfo_t[num];
        uint8_t **newReadings = new uint8_t*[num];
        memcpy(newSize, sizeOfReadings, sizeof(sizeOfReadings));
        memcpy(newPackInfo, readingInfo, sizeof(readingInfo));
        int i;
        for (i = 0; i < numOfReadings; i++) {
            newReadings[i] = new uint8_t[sizeOfReadings[i]];
            memcpy(newReadings[i], readings[i], sizeOfReadings[i]);
        }

        for (int j{i}; j < num; j++) {
            newReadings[j] = new uint8_t[4];
        }

        delete[] sizeOfReadings;
        delete[] readingInfo;
        for (int i{0}; i < numOfReadings; i++) {
            delete[] readings[i];
        }
        delete[] readings;

        readings = newReadings;
        sizeOfReadings = newSize;
        readingInfo = newPackInfo;
        numOfReadings = num;
    }
    xSemaphoreGiveRecursive(mutex);
}


