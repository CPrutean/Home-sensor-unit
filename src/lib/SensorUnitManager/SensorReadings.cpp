#include "SensorUnitManager.h"


/**
 * @breif: Default sensor unit reading constructor. Initializes reading arrays for one reading to hold one integer initially
 */
SensorUnitReadings::SensorUnitReadings() {
    readingCount = 1;
    readings = new uint8_t*[1];
    readings[0] = new uint8_t[4];

    readingSizeArray = new uint8_t;
    readingSizeArray[0] = 4; //Enough to store an int
    readingInfo = new PacketInfo_t[1];
    mutex = xSemaphoreCreateRecursiveMutex();
}


/**
 * @breif: returns the number of readings stored internally
 * @return: Returns the number of readings available
 */
int SensorUnitReadings::getReadingCount() {
    if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("FAILED TO TAKE MUTEX IN SENSOR READINGS RETURNING");
        return -1;
    }
    int num = readingCount;
    xSemaphoreGiveRecursive(mutex);
    return num;
}

/**
 * @breif: posts a reading to the internal object based on the PacketInfo parameter
 * @param data: pointer to the bytes of data we are posting to the internal object
 * @param readingSize: the size of the reading we are posting
 * @param packInfo: The information on the sensor and the index of the command we are storing
 */
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


/** 
 @breif: sets the internal reading count, if resized to larger than its original size it copies the data 
if resized to smaller resets the sotrage and assumes empty states
 @param num: number of readings we are resizing the internal memory to

*/
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

/**
 * @breif: returns a reading into a buffer based on the PacketInfo requested
 * @param buffer: the buffer we are writing the data to
 * @param bufferSize: the size of the buffer we are writing to
 * @param info: Specific sensor reading we are requesting
 */
void SensorUnitReadings::getReading(uint8_t *buffer, uint8_t bufferSize, PacketInfo_t info) {
    if (buffer == nullptr || bufferSize  == 0) {
        Serial.println("Invalid buffer values");
        return;
    }

    int i;
    bool found = false;
    for (i = 0; i < readingCount; i++) {
        if (info == readingInfo[i]) {
            found = 1;
            break;
        }
    }
    
    if (!found) {
        Serial.println("Unable to find reading");
    } else if (readingSizeArray[i] > bufferSize) {
        Serial.println("Buffer too small for readings");
    } else {
        memcpy(buffer, readings[i], bufferSize);
    }
}