#pragma once

#include "../SensorUnitManager/ManagerTypes.h"
#include <SensorUnitManager/SensorUnitManager.h>
#include <WebServer.h>

// Dashboard API for SensorUnitManager
// Provides web interface and REST API endpoints for viewing sensor data
// from all sensor units managed by the SensorUnitManager

// Initialize dashboard routes on the web server for SensorUnitManager
// Sets up HTTP routes for the dashboard HTML interface and API endpoints
//
// Routes configured:
//   GET /          - Serves the main dashboard HTML page
//   GET /dashboard - Serves the main dashboard HTML page
//   GET /api/sensors - Returns JSON with aggregated sensor data from all sensor
//   units
//
// Parameters:
//   server  - Reference to the WebServer instance to configure
//   manager - Pointer to the SensorUnitManager instance that manages sensor
//   units
//
// Note: Call this function after creating your WebServer instance but before
// server.begin()
void initDashboardAPI(WebServer &server, SensorUnitManager *manager);

// Handle sensor data API endpoint for SensorUnitManager
// Returns aggregated JSON data from all sensor units managed by the manager
//
// Response format:
//   - deviceId: MAC address of the manager device
//   - deviceType: Always "SensorUnitManager"
//   - uptime: System uptime in seconds
//   - rssi: WiFi signal strength
//   - freeMemory: Available heap memory in bytes
//   - sensorUnits: Array of sensor unit objects, each containing:
//     - unitIndex: Index of the sensor unit
//     - unitId: Unique identifier for the sensor unit
//     - status: Connection status (ONLINE, OFFLINE, ERROR, UNKNOWN)
//     - sensors: Array of sensor objects with readings
//
// Parameters:
//   server  - Reference to the WebServer instance handling the request
//   manager - Pointer to the SensorUnitManager to query for sensor data
//
// Returns: HTTP 200 with JSON payload on success, HTTP 500 on manager error
void handleSensorDataAPI(WebServer &server, SensorUnitManager *manager);
