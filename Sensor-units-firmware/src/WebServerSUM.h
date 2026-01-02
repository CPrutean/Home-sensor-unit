#pragma once
#include "global_include.h"
#include <ArduinoJson.h>
#include <WebServer.h>

// Structure to store WiFi credentials
struct WiFiCredentials {
  char ssid[32];
  char password[64];
  bool isConfigured;
};

// Initialize the Access Point web server
// apSSID: Name of the AP network (default: "SensorUnit_Config")
// apPassword: Password for the AP (default: "12345678")
void initAPWebServer(const char* apSSID = "SensorUnit_Config", const char* apPassword = "12345678", WebServer &server);

// Handle web server client requests - call this in your main loop
void handleAPWebServer(WebServer &server);

// Get stored WiFi credentials
WiFiCredentials getStoredCredentials();

// Check if device is connected to WiFi
bool isWiFiConnected();

// Get current WiFi connection status as a string
String getConnectionStatus();

void initWebsite();
