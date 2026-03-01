# Home-Sensor-Unit

A distributed home monitoring system built on ESP32 microcontrollers. Multiple sensor nodes collect environmental data and relay it to a central manager, which aggregates readings and exposes them via a local REST API and web dashboard.

## Architecture

The system is composed of four distinct firmware environments, each flashed to a separate ESP32:

```
[SensorUnit1]  ──ESP-NOW──┐
[SensorUnit2]  ──ESP-NOW──┤──► [SensorUnitManager] ──UART──► [SensorUnitWebServer]
[SensorUnitN]  ──ESP-NOW──┘           │
                                       └──► REST API / Dashboard (built-in web server)
```

### Devices

| Firmware | Role | Sensors |
|---|---|---|
| **SensorUnit1** | Leaf node | PIR motion sensor |
| **SensorUnit2** | Leaf node | DHT22 temperature & humidity |
| **SensorUnitManager** | Hub | None — aggregates data from all units |
| **SensorUnitWebServer** | Gateway | None — bridges SUM to WiFi via UART |

## Communication Protocols

### ESP-NOW (SensorUnit ↔ SensorUnitManager)

All inter-device communication uses a custom binary `Packet` struct (max 96 bytes). The header is aligned to 16 bytes so critical fields can be copied efficiently in a single operation.

**Packet types:** `PING`, `ACK`, `READING`, `POST`, `FIN`

**Data types:** `DOUBLE`, `FLOAT`, `INT`, `STRING`, `NULL`

**Sensor types:** `TEMPERATURE_AND_HUMIDITY`, `MOTION`, `BASE`

Security is enforced via ESP-NOW PMK (Pair Master Key) and per-unit LMK (Local Master Key) encryption, configured in `src/Local_config.h`.

### UART/Serial (SensorUnitManager ↔ SensorUnitWebServer)

The `PortalSUM` and `PortalWEB` classes implement a line-delimited text protocol over hardware serial (`Serial1`). Messages include a type header (`BufMsgHeader`) and pipe-separated fields:
- `SU_INFO` — sends sensor unit information
- `ADD_NEW_SU` — registers a new sensor unit
- `HANDSHAKE` — initial connection handshake
- `ERROR` — error signalling

## Concurrency

FreeRTOS tasks handle concurrent workloads on each device:

- **Packet Handler Task** (Core 0 on SUM, Core 1 on SensorUnits) — drains the `MessageQueue` and dispatches packets to the appropriate handler
- **Sensor Ping Task** (Core 1 on SUM) — pings all registered sensor units every 10 seconds

## Web Interface (SensorUnitManager)

The `DashboardAPI` component sets up an HTTP server directly on the SensorUnitManager:

| Route | Description |
|---|---|
| `GET /` | HTML dashboard page |
| `GET /dashboard` | HTML dashboard page |
| `GET /api/sensors` | JSON aggregated sensor readings from all units |

The JSON response includes device ID, uptime, RSSI, free heap memory, and a per-unit breakdown of sensor readings with status (`ONLINE`, `OFFLINE`, `ERROR`).

## Project Structure

```
src/
├── Local_config.h               # MAC addresses and encryption keys
├── SensorUnit1/main.cpp         # PIR motion sensor node
├── SensorUnit2/main.cpp         # DHT22 temperature/humidity node
├── SensorUnitManager/main.cpp   # Central hub firmware
└── lib/
    ├── Core/
    │   └── global_include.h     # Packet structs, SensorDefinition, shared types
    ├── Components/
    │   ├── MessageQueue         # FreeRTOS queue wrapper for Packet delivery
    │   └── MessageAck           # Message acknowledgement tracking
    ├── SensorUnits/
    │   ├── SensorUnits.h        # SensorUnit class (leaf node)
    │   └── Sensors.cpp          # Sensor command handlers (temp, motion, base)
    ├── SensorUnitManager/
    │   ├── SensorUnitManager.h  # SUM class (hub)
    │   ├── ManagerTypes.h       # SensorUnitInfo, SensorUnitReadings, status enum
    │   └── SensorReadings.cpp   # Thread-safe reading storage
    ├── WebComponents/
    │   ├── DashboardAPI.h       # REST API and dashboard route setup
    │   ├── DashboardAPI.cpp     # API handler implementation
    │   └── dashboard.h          # Embedded HTML for the dashboard
    └── WebServer-SUM-portal/
        ├── Portal.h             # PortalSUM and PortalWEB class declarations
        ├── PortalSUM.cpp        # UART handler for the SensorUnitManager side
        └── PortalWEB.cpp        # UART handler and WiFiManager for the web server side
```

## Build System

Built with [PlatformIO](https://platformio.org/) targeting ESP32 (Arduino framework, C++17).

```ini
# Build a specific environment
pio run -e SensorUnit1
pio run -e SensorUnit2
pio run -e SensorUnitManager
pio run -e SensorUnitWebServer

# Upload and monitor
pio run -e SensorUnitManager --target upload
pio device monitor --baud 115200
```

**Dependencies:**
- `adafruit/DHT sensor library` — temperature & humidity
- `robtillaart/PIR` — motion detection
- `blackhack/LCD_I2C` — I2C LCD display
- `bblanchon/ArduinoJson` — JSON serialization
- `tzapu/WiFiManager` — captive portal WiFi provisioning
- ESP32 built-ins: `WiFi`, `WebServer`, `esp_now`, `FreeRTOS`

## Configuration

Edit `src/Local_config.h` before flashing:

```cpp
namespace CONFIG {
  // MAC address of the SensorUnit (used on each leaf node)
  const uint8_t CUMAC[6]{ ... };

  // MAC addresses of up to MAXPEERS (6) SensorUnitManagers
  const uint8_t SUMAC[6][6]{ ... };

  // ESP-NOW encryption keys
  const char *PMKKEY{ ... };       // Pair Master Key (shared)
  const char *SUMLMKKEY{ ... };    // LMK for SensorUnit → SUM direction
  const char *SULMKKEYS[6]{ ... }; // Per-unit LMKs on the SUM side
}
```

## Lessons Learned

- **Binary over strings**: The original implementation parsed string messages between devices. Replacing this with a structured binary `Packet` format with an aligned header significantly reduced CPU overhead on the microcontrollers.
- **Design for constrained resources**: FreeRTOS task stacks, queue depths, and packet sizes are all fixed at compile time to avoid heap fragmentation on the ESP32.
- **Security at the transport layer**: Using ESP-NOW's built-in PMK/LMK encryption avoids implementing custom encryption on top of an already resource-constrained protocol.
