# Greenhouse Master v1.0 Stable

### Systems Overview
Greenhouse Master is a robust IoT monitoring solution featuring a decoupled hardware architecture. By utilizing an **Arduino Uno** for deterministic sensor acquisition and a **NodeMCU (ESP8266)** as a high-level network gateway, the system ensures data integrity even during high-traffic web requests.

---

### Technical Specifications

#### Hardware Interconnects
The system operates on a dual-MCU UART bridge. Communication is handled via a 9600-baud serial link with a dedicated logic-level shift to protect the 3.3V ESP8266 internals.

* **Climate Sensing**: DHT22 digital sensor (Air Temp/Humidity).
* **Soil Analysis**: Analog resistive probes mapped 0-100%.
* **Local Interface**: I2C LCD 1602 for real-time status updates.
* **Network Gateway**: NodeMCU hosting an asynchronous HTTP server.



#### Pin Mapping
| Component | Controller | Pin | Logic |
| :--- | :--- | :--- | :--- |
| **DHT22** | Arduino | D4 | One-Wire |
| **Soil Sensor** | Arduino | A0 | Analog |
| **LCD SDA** | Arduino | A4 | I2C |
| **LCD SCL** | Arduino | A5 | I2C |
| **TX Bridge** | Arduino | D3 | UART (to Node D1) |
| **RX Bridge** | Arduino | D2 | UART (from Node D2) |

---

### Key Features

#### Persistent Data Management
The system utilizes the **LittleFS** file system to store plant metadata in non-volatile flash memory. 
* **Immortal Configuration**: Plant names and slot assignments survive power cycles without data loss.
* **Dynamic Scalability**: Supports the creation of up to 100 software-defined plant entries.



#### Industrial UI Logic
The web interface is engineered for clarity and professional diagnostics.
* **Vertical Climate Stack**: Centralized view of air conditions.
* **Hardware Validation**: Real-time checking of physical sensor presence; unpopulated slots are explicitly labeled as "Disconnected" to distinguish software entries from hardware reality.
* **Packet Validation**: Uses start-byte `<` and end-byte `>` filtering to prevent data corruption during MCU transmission.



---

### Deployment Notes
1. **File System**: Ensure the PlatformIO environment is configured for `littlefs` to enable data persistence.
2. **Serial Handshake**: If experiencing upload failures (`stk500_getsync()`), disconnect the hardware bridge pins (D2/D3) on the Arduino to clear the UART buffer.
3. **Power**: A common ground between the Arduino and NodeMCU is mandatory for stable signal transmission.

---

**Lead Engineer**: BoredSushi  
**Release**: Saturday, February 14, 2026