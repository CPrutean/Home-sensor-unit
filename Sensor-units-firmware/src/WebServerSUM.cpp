#include "WebServerSUM.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>

// Web server on port 80
WebServer server(80);

// Preferences for storing WiFi credentials
Preferences preferences;

// Storage for WiFi credentials
WiFiCredentials wifiCreds;

// HTML template for the configuration page
const char* APSERVERHTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>WiFi Configuration</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: Arial, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      padding: 20px;
    }
    .container {
      background: white;
      border-radius: 10px;
      box-shadow: 0 10px 40px rgba(0,0,0,0.2);
      max-width: 500px;
      width: 100%;
      padding: 30px;
    }
    h1 {
      color: #333;
      margin-bottom: 10px;
      font-size: 24px;
    }
    .subtitle {
      color: #666;
      margin-bottom: 25px;
      font-size: 14px;
    }
    .section {
      margin-bottom: 25px;
    }
    .section-title {
      color: #555;
      font-size: 16px;
      font-weight: bold;
      margin-bottom: 12px;
    }
    .network-list {
      border: 1px solid #ddd;
      border-radius: 5px;
      max-height: 300px;
      overflow-y: auto;
      margin-bottom: 15px;
    }
    .network-item {
      padding: 12px 15px;
      border-bottom: 1px solid #eee;
      cursor: pointer;
      display: flex;
      justify-content: space-between;
      align-items: center;
      transition: background 0.2s;
    }
    .network-item:last-child {
      border-bottom: none;
    }
    .network-item:hover {
      background: #f5f5f5;
    }
    .network-item.selected {
      background: #e3f2fd;
      border-left: 3px solid #667eea;
    }
    .network-name {
      font-weight: 500;
      color: #333;
    }
    .network-signal {
      display: flex;
      align-items: center;
      gap: 5px;
      color: #666;
      font-size: 12px;
    }
    .signal-icon {
      font-size: 16px;
    }
    .lock-icon {
      color: #ff9800;
    }
    input[type="text"], input[type="password"] {
      width: 100%;
      padding: 12px;
      border: 1px solid #ddd;
      border-radius: 5px;
      font-size: 14px;
      margin-bottom: 15px;
    }
    input[type="text"]:focus, input[type="password"]:focus {
      outline: none;
      border-color: #667eea;
      box-shadow: 0 0 0 3px rgba(102, 126, 234, 0.1);
    }
    .btn {
      width: 100%;
      padding: 12px;
      border: none;
      border-radius: 5px;
      font-size: 16px;
      font-weight: bold;
      cursor: pointer;
      transition: all 0.3s;
    }
    .btn-primary {
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      color: white;
    }
    .btn-primary:hover {
      transform: translateY(-2px);
      box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
    }
    .btn-secondary {
      background: #f5f5f5;
      color: #333;
      margin-top: 10px;
    }
    .btn-secondary:hover {
      background: #e0e0e0;
    }
    .status {
      padding: 10px;
      border-radius: 5px;
      margin-bottom: 15px;
      display: none;
    }
    .status.success {
      background: #d4edda;
      color: #155724;
      border: 1px solid #c3e6cb;
      display: block;
    }
    .status.error {
      background: #f8d7da;
      color: #721c24;
      border: 1px solid #f5c6cb;
      display: block;
    }
    .loading {
      text-align: center;
      padding: 20px;
      color: #666;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>WiFi Configuration</h1>
    <p class="subtitle">Select a network and enter credentials to connect</p>

    <div id="status" class="status"></div>

    <div class="section">
      <div class="section-title">Available Networks</div>
      <div class="network-list" id="networkList">
        <div class="loading">Scanning for networks...</div>
      </div>
      <button class="btn btn-secondary" onclick="scanNetworks()">Refresh Networks</button>
    </div>

    <form id="wifiForm" onsubmit="submitForm(event)">
      <div class="section">
        <div class="section-title">Network Credentials</div>
        <input type="text" id="ssid" name="ssid" placeholder="Network Name (SSID)" required readonly>
        <input type="password" id="password" name="password" placeholder="Password" required>
      </div>

      <button type="submit" class="btn btn-primary">Connect to Network</button>
    </form>
  </div>

  <script>
    let selectedSSID = '';

    function scanNetworks() {
      document.getElementById('networkList').innerHTML = '<div class="loading">Scanning for networks...</div>';

      fetch('/scan')
        .then(response => response.json())
        .then(data => {
          displayNetworks(data.networks);
        })
        .catch(error => {
          document.getElementById('networkList').innerHTML = '<div class="loading">Error scanning networks</div>';
          console.error('Error:', error);
        });
    }

    function displayNetworks(networks) {
      const listDiv = document.getElementById('networkList');

      if (networks.length === 0) {
        listDiv.innerHTML = '<div class="loading">No networks found</div>';
        return;
      }

      listDiv.innerHTML = '';
      networks.forEach(network => {
        const item = document.createElement('div');
        item.className = 'network-item';
        item.onclick = () => selectNetwork(network.ssid, item);

        const signalStrength = getSignalStrength(network.rssi);
        const isSecure = network.encryption !== 'Open';

        item.innerHTML = `
          <span class="network-name">${network.ssid}</span>
          <span class="network-signal">
            ${isSecure ? '<span class="lock-icon">🔒</span>' : ''}
            <span class="signal-icon">${signalStrength.icon}</span>
            <span>${signalStrength.text}</span>
          </span>
        `;

        listDiv.appendChild(item);
      });
    }

    function getSignalStrength(rssi) {
      if (rssi >= -50) return { icon: '📶', text: 'Excellent' };
      if (rssi >= -60) return { icon: '📶', text: 'Good' };
      if (rssi >= -70) return { icon: '📡', text: 'Fair' };
      return { icon: '📡', text: 'Weak' };
    }

    function selectNetwork(ssid, element) {
      document.querySelectorAll('.network-item').forEach(item => {
        item.classList.remove('selected');
      });
      element.classList.add('selected');
      selectedSSID = ssid;
      document.getElementById('ssid').value = ssid;
    }

    function submitForm(event) {
      event.preventDefault();

      const ssid = document.getElementById('ssid').value;
      const password = document.getElementById('password').value;

      if (!ssid) {
        showStatus('Please select a network', 'error');
        return;
      }

      showStatus('Connecting to ' + ssid + '...', 'success');

      fetch('/connect', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: 'ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(password)
      })
      .then(response => response.json())
      .then(data => {
        if (data.success) {
          showStatus('Successfully connected! Device will restart...', 'success');
          setTimeout(() => {
            window.location.reload();
          }, 3000);
        } else {
          showStatus('Connection failed: ' + data.message, 'error');
        }
      })
      .catch(error => {
        showStatus('Error: ' + error.message, 'error');
        console.error('Error:', error);
      });
    }

    function showStatus(message, type) {
      const statusDiv = document.getElementById('status');
      statusDiv.textContent = message;
      statusDiv.className = 'status ' + type;
    }

    // Scan networks on page load
    window.onload = scanNetworks;
  </script>
</body>
</html>
)rawliteral";

// Function to scan WiFi networks and return as JSON
void handleScan() {
  int n = WiFi.scanNetworks();
  String json = "{\"networks\":[";

  for (int i = 0; i < n; i++) {
    if (i > 0) json += ",";
    json += "{";
    json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";

    String encryption;
    switch (WiFi.encryptionType(i)) {
      case WIFI_AUTH_OPEN:
        encryption = "Open";
        break;
      case WIFI_AUTH_WEP:
        encryption = "WEP";
        break;
      case WIFI_AUTH_WPA_PSK:
        encryption = "WPA";
        break;
      case WIFI_AUTH_WPA2_PSK:
        encryption = "WPA2";
        break;
      case WIFI_AUTH_WPA_WPA2_PSK:
        encryption = "WPA/WPA2";
        break;
      case WIFI_AUTH_WPA2_ENTERPRISE:
        encryption = "WPA2-EAP";
        break;
      default:
        encryption = "Unknown";
    }
    json += "\"encryption\":\"" + encryption + "\"";
    json += "}";
  }

  json += "]}";
  WiFi.scanDelete();

  server.send(200, "application/json", json);
}

// Function to handle WiFi connection request
void handleConnect() {
  if (!server.hasArg("ssid") || !server.hasArg("password")) {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing parameters\"}");
    return;
  }

  String ssid = server.arg("ssid");
  String password = server.arg("password");

  // Store credentials
  strncpy(wifiCreds.ssid, ssid.c_str(), sizeof(wifiCreds.ssid) - 1);
  strncpy(wifiCreds.password, password.c_str(), sizeof(wifiCreds.password) - 1);
  wifiCreds.isConfigured = true;

  // Save to preferences
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.putBool("configured", true);
  preferences.end();

  server.send(200, "application/json", "{\"success\":true,\"message\":\"Credentials saved\"}");

  // Attempt to connect
  delay(1000);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(wifiCreds.ssid, wifiCreds.password);
}

// Function to serve the main configuration page
void handleRoot() {
  server.send(200, "text/html", APSERVERHTML);
}

// Function to initialize the AP web server
void initAPWebServer(const char* apSSID = "SensorUnit_Config", const char* apPassword = "12345678") {
  // Load saved credentials
  preferences.begin("wifi", true);
  String savedSSID = preferences.getString("ssid", "");
  String savedPassword = preferences.getString("password", "");
  bool configured = preferences.getBool("configured", false);
  preferences.end();

  if (configured && savedSSID.length() > 0) {
    strncpy(wifiCreds.ssid, savedSSID.c_str(), sizeof(wifiCreds.ssid) - 1);
    strncpy(wifiCreds.password, savedPassword.c_str(), sizeof(wifiCreds.password) - 1);
    wifiCreds.isConfigured = true;
  }

  // Start WiFi in AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID, apPassword);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Setup server routes
  server.on("/", handleRoot);
  server.on("/scan", handleScan);
  server.on("/connect", HTTP_POST, handleConnect);

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

// Function to handle client requests (call this in loop)
void handleAPWebServer() {
  server.handleClient();
}

// Function to get stored WiFi credentials
WiFiCredentials getStoredCredentials() {
  return wifiCreds;
}

// Function to check if connected to WiFi
bool isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}

// Function to get current connection status
String getConnectionStatus() {
  switch (WiFi.status()) {
    case WL_CONNECTED:
      return "Connected to " + String(wifiCreds.ssid) + " (IP: " + WiFi.localIP().toString() + ")";
    case WL_NO_SSID_AVAIL:
      return "Network not found";
    case WL_CONNECT_FAILED:
      return "Connection failed - Check password";
    case WL_DISCONNECTED:
      return "Disconnected";
    default:
      return "Unknown status";
  }
}