#include "SensorUnitManager.h"

//Make space for all of the sensors, it may necessarily not have readings but ensure space is made
SensorUnitReadings::SensorUnitReadings() {
   uint8_t num = static_cast<uint8_t>(Sensors_t::BASE);
   for (int i{0};  i < num; i++) {
        packetGroupings[i] = new Packet[1];
        packetGroupings[i][0].info.sensor = static_cast<Sensors_t>(i);
        packetGroupings[i][0].info.ind = 0; 
        packetCount[i] = 0;
   }
   sensorCount = num;
}

void SensorUnitReadings::postReading(const Packet &p) {
    if (xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("Failed to take mutex");
    }  
    uint8_t sensorInd{static_cast<uint8_t>(p.info.sensor)};


    if (p.info.ind >= packetCount[sensorInd] || packetCount[sensorInd] == 0) {
        Serial.println("Invalid ind or sensor passed to post packet, check that this sensor actually exists");
    } else [[likely]] {
        packetGroupings[sensorInd][p.info.ind] = p;        
    }
    
    xSemaphoreGiveRecursive(mutex);
}


int SensorUnitReadings::getReadingCount() {
    if (xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("Failed to take mutex");
    } 
    int returnVal{0};

    for (int i{0}; i < sensorCount; i++) {
        returnVal += packetCount[i];
    }

    xSemaphoreGiveRecursive(mutex);
    return returnVal;
}

static void copyPacketBuff(Packet* dest, uint8_t destSize, Sensors_t sensor, const Packet* src = nullptr, uint8_t srcSize = 0) {
    if (destSize<srcSize) {
        Serial.println("Invalid sizes passed to copy buff packet");
        return;
    }
    int i = 0;
    if (src != nullptr) {
        for (i; i < srcSize; i++) {
            dest[i] = src[i];
        }
    }  


    for (uint8_t j{i}; j < destSize; j++) {
        dest[j].info = {sensor, j};
    }
}

void SensorUnitReadings::postNewSensor(Sensors_t sensor, uint8_t readingCount) {
    if (xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("Failed to take mutex");
    } 
    uint8_t ind = static_cast<uint8_t>(sensor);
    if (packetCount[ind] == readingCount  || readingCount < packetCount[ind]) {
        Serial.println("Invalid reading count or sensor");
    } else if (ind >= sensorCount) {
        Packet** newPtr = new Packet*[ind+1];
        int i;

        for (i = 0; i < sensorCount; i++) {
            newPtr[i] = packetGroupings[i];
        }

        for (int j{i}; j < ind-1; j++) {
            newPtr[j] = new Packet[1];
        }
        newPtr[ind] = new Packet[readingCount];
        copyPacketBuff(newPtr[ind], readingCount, sensor);

        delete[] packetGroupings;
        packetGroupings = newPtr;

    } else if (readingCount != packetCount[ind]) {
        Packet* newPtr = new Packet[readingCount];
        copyPacketBuff(newPtr, readingCount, sensor, packetGroupings[ind], packetCount[ind]);

        delete[] packetGroupings[ind];
        packetGroupings[ind] = newPtr;
    }     
    xSemaphoreGiveRecursive(mutex);
}



Packet& SensorUnitReadings::getReading(PacketInfo_t packet) {
    if (xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("Failed to take mutex");
    } 
    uint8_t ind = static_cast<uint8_t>(packet.sensor);

    Packet& ref = packetGroupings[ind][packet.ind];

    xSemaphoreGiveRecursive(mutex);
    return ref;
}


