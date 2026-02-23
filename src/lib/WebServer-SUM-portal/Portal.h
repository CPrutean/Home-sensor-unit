#pragma once
#include <Core/global_include.h>
#include <SensorUnitManager/SensorUnitManager.h>
#include <memory>
#include <cstring>
#include <WebServer.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include <Preferences.h>

// Seperate packets when sending packets to
const char PACKET_SEPERATOR[2]{'.', '\0'};

// Seperates different message sections
const char END_OF_SECTION[2]{'|', '\0'};

// Seperates end of messages sent between devices
const char END_OF_LINE[2]{'\n', '\0'};


//Just so we can cast values back into readable char values
enum class BufMsgHeader: uint8_t {SU_INFO = static_cast<uint8_t>('a'), ADD_NEW_SU, ERROR, HANDSHAKE};



//SensorUnitManager end of the portal
class PortalSUM final {

private:
    std::unique_ptr<char[]> buffer;
    size_t capacity;
    size_t bufLength;
    //Serial for the RX and TX pins
    HardwareSerial uartBuffer;
    Preferences prefs;
    SensorUnitManager &sum;
    bool handshakeReceived{false};
    void sendHandshake();
    void clearBuffer();

public:
    PortalSUM(SensorUnitManager& sumRef) : buffer{std::make_unique<char[]>(2000)}, capacity{2000}, bufLength{0}, uartBuffer{Serial1}, sum{sumRef} {

    }

    void handleBufferLine();

    void sendSensorUnitInfo(SensorUnitInfo* inf = nullptr);
};


//WebServer end of the buffer portal
//This class will also act similarly to the other devices, handling internal api calls to different local APIS
class PortalWEB final {

private:
    std::unique_ptr<char[]> buffer;
    size_t capacity;
    size_t bufLength;
    WiFiManager wm;
    WebServer server;
    std::array<SensorUnitInfo, MAXPEERS>  suInfo;
    Preferences pref;

    //Serial for the RX and TX pins
    HardwareSerial uartBuffer;
    void sendHandshake();
    bool handshakeReceived{false};
    void clearBuffer();

public:
    //Constructor is going to have all of the
    PortalWEB() : buffer{std::make_unique<char[]>(200)}, capacity{200}, bufLength{0}, uartBuffer{Serial1} {
        //Delete the values
        // wm.resetSettings();
        wm.setClass("invert");
        bool result = wm.autoConnect("Home-Sens-AP", "password");

        if (!result) {
            Serial.println("Failed to connect");
        } else {
            Serial.println("Successfully connected");
        }

        server.begin(80);


    }
    void handleBufferLine();
    void sendSensorUnit(const uint8_t* mac);
};

void writeSuInf(char* buffer, size_t capacity, const SensorUnitInfo& suInf);
void readSuInfFromBuffer(SensorUnitInfo& dest, const char* src, size_t start, size_t end);
