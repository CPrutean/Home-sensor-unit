#pragma once

#include <WebServer.h>
#include <SensorUnitManager/SensorUnitManager.h>
#include "../SensorUnitManager/ManagerTypes.h"

// Initialize dashboard routes on the web server for SensorUnitManager
// This will set up routes for the dashboard HTML and API endpoints
// Call this function after creating your WebServer instance
void initDashboardAPI(WebServer &server, SensorUnitManager *manager);

// Handle sensor data API endpoint for SensorUnitManager
// Returns aggregated data from all sensor units managed by the manager
void handleSensorDataAPI(WebServer &server, SensorUnitManager *manager);
