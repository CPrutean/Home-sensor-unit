#pragma once
#include <WiFi.h>
#include <WebServer.h>
#include "WebServerSUM.h"
#include "global_include.h"

class SUMWebsite {
public:
    SUMWebsite(WiFiCredentials wifiCreds);
    unsigned long long prevTime{};
private:
    WebServer server;
};