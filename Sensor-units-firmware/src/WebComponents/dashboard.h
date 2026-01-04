#pragma once

// HTML template for the sensor unit dashboard
const char* DASHBOARD_HTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Sensor Unit Dashboard</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }

    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
      min-height: 100vh;
      padding: 20px;
    }

    .header {
      text-align: center;
      color: white;
      margin-bottom: 30px;
    }

    .header h1 {
      font-size: 32px;
      margin-bottom: 8px;
    }

    .header .status {
      display: inline-block;
      padding: 6px 15px;
      background: rgba(255, 255, 255, 0.2);
      border-radius: 20px;
      font-size: 14px;
      margin-top: 10px;
    }

    .status.online {
      background: rgba(76, 175, 80, 0.3);
    }

    .status.offline {
      background: rgba(244, 67, 54, 0.3);
    }

    .container {
      max-width: 1200px;
      margin: 0 auto;
    }

    .sensors-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 20px;
      margin-bottom: 30px;
    }

    .sensor-unit-container {
      background: rgba(255, 255, 255, 0.95);
      border-radius: 15px;
      padding: 20px;
      margin-bottom: 25px;
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2);
    }

    .sensor-unit-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 20px;
      padding-bottom: 15px;
      border-bottom: 2px solid #e0e0e0;
    }

    .sensor-unit-title {
      font-size: 22px;
      font-weight: 700;
      color: #1e3c72;
    }

    .unit-status {
      padding: 8px 16px;
      border-radius: 20px;
      font-size: 14px;
      font-weight: 600;
    }

    .unit-status.online {
      background: rgba(76, 175, 80, 0.2);
      color: #2e7d32;
    }

    .unit-status.offline {
      background: rgba(244, 67, 54, 0.2);
      color: #c62828;
    }

    .unit-status.error {
      background: rgba(255, 152, 0, 0.2);
      color: #ef6c00;
    }

    .sensor-card {
      background: white;
      border-radius: 15px;
      padding: 25px;
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2);
      transition: transform 0.3s, box-shadow 0.3s;
    }

    .sensor-card:hover {
      transform: translateY(-5px);
      box-shadow: 0 15px 40px rgba(0, 0, 0, 0.3);
    }

    .sensor-header {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 20px;
    }

    .sensor-title {
      font-size: 18px;
      font-weight: 600;
      color: #333;
    }

    .sensor-icon {
      font-size: 32px;
    }

    .sensor-value {
      font-size: 48px;
      font-weight: bold;
      color: #2a5298;
      margin-bottom: 10px;
    }

    .sensor-unit {
      font-size: 24px;
      color: #666;
      margin-left: 5px;
    }

    .sensor-label {
      font-size: 14px;
      color: #999;
      text-transform: uppercase;
      letter-spacing: 1px;
    }

    .sensor-timestamp {
      font-size: 12px;
      color: #bbb;
      margin-top: 15px;
      padding-top: 15px;
      border-top: 1px solid #eee;
    }

    .info-panel {
      background: white;
      border-radius: 15px;
      padding: 25px;
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2);
    }

    .info-row {
      display: flex;
      justify-content: space-between;
      padding: 12px 0;
      border-bottom: 1px solid #eee;
    }

    .info-row:last-child {
      border-bottom: none;
    }

    .info-label {
      color: #666;
      font-weight: 500;
    }

    .info-value {
      color: #333;
      font-family: monospace;
    }

    .loading {
      text-align: center;
      padding: 40px;
      color: white;
      font-size: 18px;
    }

    .error {
      background: #f44336;
      color: white;
      padding: 15px;
      border-radius: 10px;
      margin-bottom: 20px;
      display: none;
    }

    .refresh-btn {
      position: fixed;
      bottom: 30px;
      right: 30px;
      width: 60px;
      height: 60px;
      border-radius: 50%;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      border: none;
      color: white;
      font-size: 24px;
      cursor: pointer;
      box-shadow: 0 5px 20px rgba(102, 126, 234, 0.4);
      transition: transform 0.3s, box-shadow 0.3s;
    }

    .refresh-btn:hover {
      transform: scale(1.1);
      box-shadow: 0 8px 30px rgba(102, 126, 234, 0.6);
    }

    .refresh-btn:active {
      transform: scale(0.95);
    }

    @keyframes spin {
      from { transform: rotate(0deg); }
      to { transform: rotate(360deg); }
    }

    .refresh-btn.spinning {
      animation: spin 1s linear infinite;
    }
  </style>
</head>
<body>
  <div class="header">
    <h1>Sensor Unit Manager Dashboard</h1>
    <span class="status" id="connectionStatus">Connecting...</span>
  </div>

  <div class="container">
    <div id="errorMessage" class="error"></div>

    <div class="sensors-grid" id="sensorsGrid">
      <div class="loading">Loading sensor data...</div>
    </div>

    <div class="info-panel">
      <h2 style="margin-bottom: 20px; color: #333;">System Information</h2>
      <div class="info-row">
        <span class="info-label">Device ID</span>
        <span class="info-value" id="deviceId">Loading...</span>
      </div>
      <div class="info-row">
        <span class="info-label">Uptime</span>
        <span class="info-value" id="uptime">Loading...</span>
      </div>
      <div class="info-row">
        <span class="info-label">WiFi RSSI</span>
        <span class="info-value" id="rssi">Loading...</span>
      </div>
      <div class="info-row">
        <span class="info-label">Free Memory</span>
        <span class="info-value" id="freeMemory">Loading...</span>
      </div>
    </div>
  </div>

  <button class="refresh-btn" onclick="refreshData()" id="refreshBtn">↻</button>

  <script>
    let autoRefreshInterval;

    function formatTimestamp(timestamp) {
      const date = new Date(timestamp * 1000);
      return date.toLocaleTimeString();
    }

    function formatUptime(seconds) {
      const days = Math.floor(seconds / 86400);
      const hours = Math.floor((seconds % 86400) / 3600);
      const minutes = Math.floor((seconds % 3600) / 60);
      const secs = seconds % 60;

      let result = [];
      if (days > 0) result.push(days + 'd');
      if (hours > 0) result.push(hours + 'h');
      if (minutes > 0) result.push(minutes + 'm');
      result.push(secs + 's');

      return result.join(' ');
    }

    function updateSensorDisplay(data) {
      const grid = document.getElementById('sensorsGrid');
      grid.innerHTML = '';

      if (!data.sensorUnits || data.sensorUnits.length === 0) {
        grid.innerHTML = '<div class="loading">No sensor units connected</div>';
        return;
      }

      // Iterate through each sensor unit
      data.sensorUnits.forEach(sensorUnit => {
        // Create container for this sensor unit
        const unitContainer = document.createElement('div');
        unitContainer.className = 'sensor-unit-container';

        // Create header for sensor unit
        const unitHeader = document.createElement('div');
        unitHeader.className = 'sensor-unit-header';

        const unitTitle = document.createElement('div');
        unitTitle.className = 'sensor-unit-title';
        unitTitle.textContent = sensorUnit.unitId || 'Sensor Unit ' + sensorUnit.unitIndex;

        const statusBadge = document.createElement('span');
        statusBadge.className = 'unit-status ' + (sensorUnit.status || 'unknown').toLowerCase();
        statusBadge.textContent = '● ' + (sensorUnit.status || 'UNKNOWN');

        unitHeader.appendChild(unitTitle);
        unitHeader.appendChild(statusBadge);
        unitContainer.appendChild(unitHeader);

        // Create grid for sensors in this unit
        const sensorsGrid = document.createElement('div');
        sensorsGrid.className = 'sensors-grid';

        // Process each sensor in this unit
        if (sensorUnit.sensors && sensorUnit.sensors.length > 0) {
          sensorUnit.sensors.forEach(sensor => {
            if (sensor.type === 'TEMPERATURE_AND_HUMIDITY') {
              if (sensor.readings && sensor.readings.length >= 2) {
                const tempReading = sensor.readings.find(r => r.name === 'Temperature');
                const humReading = sensor.readings.find(r => r.name === 'Humidity');

                if (tempReading) {
                  const tempCard = createSensorCard(sensor.name + ' - Temperature', tempReading.value, '°C', '🌡️', tempReading.timestamp);
                  sensorsGrid.appendChild(tempCard);
                }

                if (humReading) {
                  const humCard = createSensorCard(sensor.name + ' - Humidity', humReading.value, '%', '💧', humReading.timestamp);
                  sensorsGrid.appendChild(humCard);
                }
              }
            } else if (sensor.type === 'MOTION') {
              const displayValue = sensor.readings && sensor.readings[0] ? (sensor.readings[0].value ? 'Detected' : 'Clear') : 'No Data';
              const timestamp = sensor.readings && sensor.readings[0] ? sensor.readings[0].timestamp : null;

              const card = document.createElement('div');
              card.className = 'sensor-card';
              card.innerHTML = `
                <div class="sensor-header">
                  <span class="sensor-title">${sensor.name}</span>
                  <span class="sensor-icon">🚶</span>
                </div>
                <div class="sensor-value" style="font-size: 32px;">${displayValue}</div>
                <div class="sensor-label">Motion Status</div>
                ${timestamp ? `<div class="sensor-timestamp">Updated: ${formatTimestamp(timestamp)}</div>` : ''}
              `;
              sensorsGrid.appendChild(card);
            }
          });
        } else {
          sensorsGrid.innerHTML = '<div class="loading">No sensors configured</div>';
        }

        unitContainer.appendChild(sensorsGrid);
        grid.appendChild(unitContainer);
      });
    }

    function createSensorCard(name, value, unit, icon, timestamp) {
      const card = document.createElement('div');
      card.className = 'sensor-card';
      card.innerHTML = `
        <div class="sensor-header">
          <span class="sensor-title">${name}</span>
          <span class="sensor-icon">${icon}</span>
        </div>
        <div>
          <span class="sensor-value">${value !== null && value !== undefined ? value.toFixed(1) : '--'}</span>
          <span class="sensor-unit">${unit}</span>
        </div>
        <div class="sensor-label">Current Reading</div>
        ${timestamp ? `<div class="sensor-timestamp">Updated: ${formatTimestamp(timestamp)}</div>` : ''}
      `;
      return card;
    }

    function updateSystemInfo(data) {
      document.getElementById('deviceId').textContent = data.deviceId || 'Unknown';
      document.getElementById('uptime').textContent = data.uptime ? formatUptime(data.uptime) : '--';
      document.getElementById('rssi').textContent = data.rssi ? data.rssi + ' dBm' : '--';
      document.getElementById('freeMemory').textContent = data.freeMemory ? (data.freeMemory / 1024).toFixed(1) + ' KB' : '--';
    }

    function updateConnectionStatus(isConnected) {
      const statusEl = document.getElementById('connectionStatus');
      if (isConnected) {
        statusEl.textContent = '● Online';
        statusEl.className = 'status online';
      } else {
        statusEl.textContent = '● Offline';
        statusEl.className = 'status offline';
      }
    }

    function showError(message) {
      const errorEl = document.getElementById('errorMessage');
      errorEl.textContent = message;
      errorEl.style.display = 'block';
      setTimeout(() => {
        errorEl.style.display = 'none';
      }, 5000);
    }

    function refreshData() {
      const btn = document.getElementById('refreshBtn');
      btn.classList.add('spinning');

      fetch('/api/sensors')
        .then(response => {
          if (!response.ok) throw new Error('Network response was not ok');
          return response.json();
        })
        .then(data => {
          updateSensorDisplay(data);
          updateSystemInfo(data);
          updateConnectionStatus(true);
          btn.classList.remove('spinning');
        })
        .catch(error => {
          console.error('Error fetching sensor data:', error);
          showError('Failed to fetch sensor data: ' + error.message);
          updateConnectionStatus(false);
          btn.classList.remove('spinning');
        });
    }

    // Initial load
    refreshData();

    // Auto-refresh every 5 seconds
    autoRefreshInterval = setInterval(refreshData, 5000);

    // Stop auto-refresh when page is not visible
    document.addEventListener('visibilitychange', () => {
      if (document.hidden) {
        clearInterval(autoRefreshInterval);
      } else {
        autoRefreshInterval = setInterval(refreshData, 5000);
        refreshData();
      }
    });
  </script>
</body>
</html>
)rawliteral";
