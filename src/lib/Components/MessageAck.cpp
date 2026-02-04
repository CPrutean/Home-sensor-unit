#include "MessageAck.h"
#include <SensorUnitManager/SensorUnitManager.h>
#define MAXTIMEOUT 10000


uint8_t MessageAck::getSuArrInd(unsigned long id) {
    if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("Failed to take mutex");
        return;
    } 
    bool found = false;
    int i;
    for (i = 0; i < suCount; i++) {
        if (id == idArray[i]) {
            found = 1;
            break;
        }
    }

    uint8_t temp = found ? i : 255;
    xSemaphoreGiveRecursive(mutex);
    return temp;
}

void MessageAck::addSensorUnit(unsigned long suID) {
    if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("Failed to take mutex");
        return;
    } else {
        idArray[suCount++] = suID;
    }

    xSemaphoreGiveRecursive(mutex);
}


void MessageAck::expectPacket(unsigned long suID) {
    if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("Failed to take mutex");
        return;
    }
    uint8_t ind;

    if ((ind = getSuArrInd(suID)) != 255) {
        packetsRequested[ind]++;
    } else {
        Serial.println("Invalid su array index");
    }
   xSemaphoreGiveRecursive(mutex);
    
}


void MessageAck::packetRecived(unsigned long suID) {
    if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("Failed to take mutex");
        return;
    } 
    
    uint8_t ind;
    if ((ind = getSuArrInd(suID)) != 255) {
        packetsRecieved[ind]++;
    } else {
        Serial.println("Invalid su array index");
    }

    xSemaphoreGiveRecursive(mutex);
}


double MessageAck::getPacketDropPercentage(unsigned long suID) {
    if (xSemaphoreTakeRecursive(mutex, portMAX_DELAY) != pdTRUE) {
        Serial.println("Failed to take mutex");
        return;
    } 

    uint8_t ind;
    double temp;
    if ((ind = getSuArrInd(suID)) != 255) {
        temp = static_cast<double>(packetsRecieved[ind])/static_cast<double>(packetsRequested[ind]);
    } else {
        Serial.println("Invalid su array index");
        temp = -1;
    }
    xSemaphoreGiveRecursive(mutex);
    return temp;
}
  
/** 
@breif: destructor frees internal ackListItem array
*/
MessageAck::~MessageAck() {
}