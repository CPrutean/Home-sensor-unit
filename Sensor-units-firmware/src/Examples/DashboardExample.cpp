/*
 * Example usage of the Sensor Unit Manager Dashboard
 *
 * This file demonstrates how to integrate the dashboard into your SensorUnitManager code.
 * The dashboard displays aggregate data from all sensor units being managed.
 * Copy the relevant parts into your main sketch (.ino file) or include this in your project.
 */

#include <Arduino.h>
#include <WebServer.h>
#include <WebComponents/DashboardAPI.h>
#include <SensorUnitManager/SensorUnitManager.h>

// Create web server instance (default port 80 for HTTP)
WebServer server(80);

// Your sensor unit manager pointer (assuming you already have this set up)
extern SensorUnitManager *sensUnitMngr;

void setup() {
  Serial.begin(115200);

  // ... your existing SensorUnitManager initialization code ...

  // Initialize WiFi Access Point for the dashboard
  // The SensorUnitManager should host the dashboard as a central aggregator
  WiFi.softAP("SensorUnitManager-Dashboard", "password123");

  // Alternatively, connect to existing WiFi network:
  // WiFi.begin("your-ssid", "your-password");
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }

  Serial.println("\nWiFi Access Point started!");
  Serial.print("Dashboard IP address: ");
  Serial.println(WiFi.softAPIP());  // Use WiFi.localIP() if connected to network

  // Initialize dashboard with SensorUnitManager
  // This sets up routes for HTML dashboard and API endpoints
  initDashboardAPI(server, sensUnitMngr);

  // Start the web server
  server.begin();
  Serial.println("Dashboard server started!");
  Serial.println("Access dashboard at: http://" + WiFi.softAPIP().toString());
  Serial.println("The dashboard will show aggregated data from all connected sensor units");
}

void loop() {
  // Handle web server requests
  server.handleClient();

  // ... your existing SensorUnitManager message handling code ...
  // The dashboard will automatically display updated sensor data from all units

  delay(10);
}
