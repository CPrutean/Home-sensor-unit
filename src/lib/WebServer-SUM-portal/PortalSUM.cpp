#include "Portal.h"
void writeSuInf(char* buffer, size_t capacity, const SensorUnitInfo& suInf) {
    if (buffer == nullptr || capacity <= 0) {
        Serial.println("Invalid write state");
        return;
    }

    buffer[0] = static_cast<char>(BufMsgHeader::SU_INFO);
    strncat(buffer, END_OF_SECTION, capacity);
    std::copy(&suInf, &suInf+sizeof(SensorUnitInfo), buffer+2);
    buffer[2+sizeof(SensorUnitInfo)+1] = '\0';
}

void readSuInfFromBuffer(SensorUnitInfo& dest, const char* src, size_t start, size_t end) {
    if (src == nullptr || start >= end) {
        Serial.println("Invalid pointer state");
        return;
    }
    std::copy(src+start, src+start+end, &dest);
}





void PortalSUM::sendHandshake() {
    snprintf(buffer.get(), capacity, "%c", static_cast<char>(BufMsgHeader::HANDSHAKE));
    strncat(buffer.get(), END_OF_SECTION, capacity);
    strncat(buffer.get(), END_OF_LINE, capacity);
    buffer.get()[3] = '\0';
    uartBuffer.print(buffer.get());
    clearBuffer();
}

void PortalSUM::sendSensorUnitInfo(SensorUnitInfo* inf) {
    if (inf == nullptr) {
        Serial.println("Invalid sensorUnitInfo was sent");
        return;
    }
    buffer.get()[0] = static_cast<char>(BufMsgHeader::SU_INFO);
    buffer.get()[1] = static_cast<char>(END_OF_SECTION[0]);
    std::copy(inf, inf+sizeof(SensorUnitInfo), buffer.get()+2);
    buffer.get()[2+sizeof(SensorUnitInfo)] = '\0';
    uartBuffer.print(buffer.get());
    clearBuffer();

}





void PortalSUM::clearBuffer() {
    std::memset(buffer.get(), 0, capacity);
}

void PortalSUM::handleBufferLine() {
    //Read until the end of line seperator
    String str{uartBuffer.readStringUntil(END_OF_LINE[0])};

    BufMsgHeader head = static_cast<BufMsgHeader>(str.charAt(0));
    str = str.substring(2);
    const char* cStr = str.c_str();
    //Portal should never be in charge of updating su info
    switch (head) {
    case(BufMsgHeader::ADD_NEW_SU):
        uint8_t mac[6]{};
        std::copy(cStr, cStr+6, mac);
        sum.addNewSU(mac);
        break;
    case(BufMsgHeader::ERROR):
        Serial.println("Error: "+str);
        break;
    case(BufMsgHeader::HANDSHAKE):
        handshakeReceived = true;
        break;
    default:
        Serial.println("Invalid sensor unit info");
        String err{static_cast<char>(BufMsgHeader::ERROR) + END_OF_SECTION};
        err += "INVALID MESSAGE TYPE";
        uartBuffer.println(err);
        return;
    };

}


