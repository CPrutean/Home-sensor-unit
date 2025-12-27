#include "SensorUnitManager.h"

SensorUnitReadings::SensorUnitReadings() {
    numOfReadings = 1;
    readings = new uint8_t*[1];
    readings[0] = new uint8_t[4];

    sizeOfReadings = new uint8_t;
    sizeOfReadings[0] = 4; //Enough to store an int 
    readingInfo = new PacketInfo_t[1];
}