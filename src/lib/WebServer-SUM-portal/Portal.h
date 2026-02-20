#include <Core/global_include.h>
#include <SensorUnitManager/ManagerTypes.h>
#include <memory>
#include <cstring>
#include <WebServer.h>

//Seperates packets when sending packets to
const char PACKET_SEPERATOR[2]{'.', '\0'};
//Seperates different message sections
const char END_OF_SECTION[2]{'|', '\0'};
//Seperates end of messages sent between devices
const char END_OF_CHUNK[2]{'\n', '\0'};

//SensorUnitManager end of the portal
class PortalSUM {
private:
    std::unique_ptr<char[]> buffer ;
    size_t capacity;
    size_t bufLength;

public:
    PortalSUM() : buffer{std::make_unique<char[]>(500)}, capacity{500}, bufLength{0} {

    }

    void handleChunk() {

    }

    void sendSensorUnitInfo(SensorUnitInfo& inf) {

    }
};


//WebServer end of the buffer portal
class PortalWEB {
private:
    std::unique_ptr<char[]> buffer;
    size_t capacity;
    size_t bufLength;
    WebServer server;
public:
    PortalWEB() : buffer{std::make_unique<char[]>(1000)}, capacity{1000}, bufLength{0} {
        server.begin(80);
    }

    void handleChunk() {

    }

    void sendNewSensorUnit() {

    }
};
