#pragma once
#include <Core/global_include.h>
#include <ArduinoJson.h>
#include <WebServer.h>

// WebServerSUM - WiFi Configuration Web Server for SensorUnitManager
// Provides a captive portal-style web interface for configuring WiFi credentials
// on ESP32-based SensorUnitManager devices. The server runs in Access Point (AP)
// mode, allowing users to connect directly to the device and configure its WiFi settings.

// Structure to store WiFi credentials
// Used for both runtime storage and persistent storage via Preferences API
struct WiFiCredentials {
  char ssid[32];        // Network SSID (max 31 chars + null terminator)
  char password[64];    // Network password (max 63 chars + null terminator)
  bool isConfigured;    // Flag indicating if credentials have been set
};

// Initialize the Access Point web server
// Creates a WiFi access point and starts a web server for WiFi configuration
//
// The server provides a web interface with the following features:
//   - Network scanning to show available WiFi networks
//   - Interactive network selection with signal strength indicators
//   - Credential entry form with validation
//   - Persistent storage of credentials using ESP32 Preferences API
//
// HTTP Routes configured:
//   GET  /         - Serves the main configuration HTML page
//   GET  /scan     - Returns JSON list of available WiFi networks
//   POST /connect  - Accepts WiFi credentials and attempts connection
//
// Parameters:
//   apSSID     - Name of the AP network (e.g., "SensorUnit_Config")
//   apPassword - Password for the AP (must be at least 8 characters)
//   server     - Reference to the WebServer instance to configure
//
// Note: Previously saved credentials are automatically loaded from Preferences
//       and the device will attempt to connect to the saved network
void initAPWebServer(const char* apSSID, const char* apPassword, WebServer &server);

// Handle web server client requests
// Processes incoming HTTP requests from connected clients
//
// This function must be called repeatedly in your main loop to handle
// client connections and serve web pages/API responses
//
// Parameters:
//   server - Reference to the WebServer instance to handle
//
// Usage example:
//   void loop() {
//     handleAPWebServer(server);
//   }
void handleAPWebServer(WebServer &server);

// Get stored WiFi credentials
// Retrieves the currently stored WiFi credentials from memory
//
// Returns: WiFiCredentials struct containing:
//   - ssid: The network SSID
//   - password: The network password
//   - isConfigured: Whether credentials have been set
//
// Note: This returns the in-memory copy. To check persistent storage,
//       use the Preferences API directly
WiFiCredentials getStoredCredentials();

// Check if device is connected to WiFi
// Determines whether the device has an active WiFi connection
//
// Returns: true if connected to a WiFi network, false otherwise
//
// Note: This checks the current connection status, not just whether
//       credentials are configured
bool isWiFiConnected();

// Get current WiFi connection status as a string
// Returns a human-readable status message about the WiFi connection
//
// Possible return values:
//   - "Connected to [SSID] (IP: [address])" - Successfully connected
//   - "Network not found" - Configured SSID not available
//   - "Connection failed - Check password" - Authentication error
//   - "Disconnected" - Not connected to any network
//   - "Unknown status" - Unexpected WiFi status
//
// Returns: String describing the current connection status
//
// Usage: Useful for debugging and displaying status to users
String getConnectionStatus();

// Initialize website components
// Reserved for future use - currently not implemented
void initWebsite();
