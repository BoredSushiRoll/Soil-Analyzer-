# Soil Telemetry Node (Hybrid Architecture)

**Status:** `[IN DEVELOPMENT]`
**Role:** Telemetry Endpoint
**Protocol:** Serial Bridge (Uno <-> ESP8266)

## System Architecture
This system utilizes a "Split-Brain" architecture to bypass the voltage limitations of the ESP8266 and the lack of network stack on the Arduino Uno.

### 1. Primary Controller (Arduino Uno)
* **Role:** Sensor Interface & Timing Logic
* **Voltage:** 5V Logic
* **Sensors:**
    * Capacitive Soil Moisture (Analog `A0`) - Corrosion resistant.
    * DHT11/22 (Digital `Pin 4`) - Ambient Temp/Humidity.
* **Output:**
    * Local OLED (I2C) - *Optional*
    * Serial Bridge (`SoftwareSerial` on Pins 2/3) -> Payload transmission.

### 2. Network Gateway (ESP8266)
* **Role:** Wi-Fi Modem / MQTT Publisher
* **Voltage:** 3.3V Logic (Requires level shifting or voltage divider from Uno).
* **Input:** Receives JSON/String payloads via UART from Uno.

## Pin Map (Draft)

| Component | Uno Pin | Notes |
| :--- | :--- | :--- |
| **DHT Sensor** | `D4` | Requires pull-up resistor (usually built-in on modules) |
| **Soil Sensor** | `A0` | Analog Read (0-1023) |
| **ESP RX** | `D3` | SoftwareSerial TX from Uno |
| **ESP TX** | `D2` | SoftwareSerial RX to Uno |
| **OLED SDA** | `A4` | I2C Data |
| **OLED SCL** | `A5` | I2C Clock |

## Environment
* **IDE:** VS Code + PlatformIO
* **Framework:** Arduino
* **Baud Rate:** 9600 (Inter-chip communication)