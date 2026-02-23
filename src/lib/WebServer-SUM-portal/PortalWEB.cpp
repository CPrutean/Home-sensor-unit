#include "Portal.h"



void PortalWEB::sendHandshake() {
    snprintf(buffer.get(), capacity, "%c", static_cast<char>(BufMsgHeader::HANDSHAKE));
    strncat(buffer.get(), END_OF_SECTION, capacity);
    strncat(buffer.get(), END_OF_LINE, capacity);
    buffer.get()[3] = '\0';
    uartBuffer.print(buffer.get());
    clearBuffer();
}



void PortalWEB::clearBuffer() {
    std::memset(buffer.get(), 0, capacity);
}

void PortalWEB::handleBufferLine() {
    String str{uartBuffer.readStringUntil('\n')};
    BufMsgHeader header;
    header = static_cast<BufMsgHeader>(str.charAt(0));
    str = str.substring(2);
    if (header == BufMsgHeader::SU_INFO) {
        const char* cStr = str.c_str();
        unsigned long suID;
        //Copy the suID so we can place the sensor unit readings in the proper address
        offsetof(SensorUnitInfo, SensorUnitID);
        std::copy(cStr+offsetof(SensorUnitInfo, SensorUnitID), cStr+offsetof(SensorUnitInfo, SensorUnitID)+sizeof(unsigned long), &suID);
        int i;
        bool updated = false;
        for (i = 0; i < MAXPEERS; i++) {
            if (suInfo[i].SensorUnitID == suID) {
                //Update old info
                std::copy(cStr, cStr+sizeof(SensorUnitInfo), &suInfo[i]);
                updated = true;
                break;
            } else if (suInfo[i].SensorUnitID == 0) {
                //Add new Sensor Unit
                std::copy(cStr, cStr+sizeof(SensorUnitInfo), &suInfo[i]);
                updated = true;
                break;
            }
        }

        if (!updated) {
            String s{""};
            s += static_cast<char>(BufMsgHeader::ERROR);
            s += END_OF_SECTION;
            s += "INVALID SENSOR UNIT INFO SENT";
            uartBuffer.println(s);
        }

    } else if (header == BufMsgHeader::HANDSHAKE) {
        handshakeReceived = true;
    } else if (header == BufMsgHeader::ERROR) {
        Serial.printf("Error received: %s\n", str.c_str());
    } else {
        String str{""};
        str += static_cast<char>(BufMsgHeader::ERROR);
        str += END_OF_SECTION;
        str += "Invalid message type sent";
    }

}



void PortalWEB::sendSensorUnit(const uint8_t* mac) {
    buffer.get()[0] = static_cast<char>(BufMsgHeader::ADD_NEW_SU);
    buffer.get()[1] = static_cast<char>(END_OF_SECTION[0]);
    std::copy(mac, mac+6, buffer.get()+2);
    buffer.get()[8] = '\n';
    buffer.get()[9] = '\0';
    uartBuffer.print(buffer.get());
    clearBuffer();
}


