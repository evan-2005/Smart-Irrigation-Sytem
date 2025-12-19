# Smart Irrigation System (AquaSense)

## Project Overview

The **Smart Irrigation System** is an IoT-enabled solution designed to monitor soil moisture and light levels, automate water delivery, and provide both offline and online control for small- to medium-scale gardens. The system combines Arduino-based sensing and control with ESP8266 WiFi connectivity and the Blynk cloud platform, allowing real-time remote monitoring and manual override via a mobile app.

Key features:

- Real-time soil moisture and light monitoring.
- Automatic irrigation with threshold-based logic.
- Local display via I2C LCD for offline operation.
- Remote monitoring and control using Blynk cloud.
- Token-based authentication ensuring secure cloud interaction.
- Buzzer and LED alerts for abnormal conditions (e.g., dry soil).

# Smart Irrigation System

## Project Overview

The **Smart Irrigation System** is an IoT-enabled solution designed to monitor soil moisture and light levels, automate water delivery, and provide both offline and online control for small- to medium-scale gardens. The system combines Arduino-based sensing and control with ESP8266 WiFi connectivity and the Blynk cloud platform, allowing real-time remote monitoring and manual override via a mobile app.

Key features:

- Real-time soil moisture and light monitoring.
- Automatic irrigation with threshold-based logic.
- Local display via I2C LCD for offline operation.
- Remote monitoring and control using Blynk cloud.
- Token-based authentication ensuring secure cloud interaction.
- Buzzer and LED alerts for abnormal conditions (e.g., dry soil).

---

## System Architecture

**Architecture Overview:**
## System Architecture

**Architecture Overview:**

[ Soil Sensor / LDR / DHT ] ---> [ Arduino Uno ]
|
| UART (SoftwareSerial)
v
[ ESP8266 WiFi Shield ]
|
| WiFi (Encrypted / Token-Based)
v
[ Blynk Cloud ]
|
v
[ Mobile App ]


- **Offline Layer:** Arduino reads sensor data, controls the pump relay, and updates the LCD.
- **Network Layer:** UART serial communication connects Arduino to ESP8266.
- **Application Layer:** Blynk app receives sensor data and sends control commands.
- **Security Layer:** Blynk uses token-based authentication for all cloud interactions.
- **Responsible Design:** The offline LCD can be used as the main display in WiFi downtime scenarios.

---

## Hardware Components

| Component                  | Function |
|-----------------------------|---------|
| Arduino Uno                 | Core controller for sensors and relay |
| ESP8266 WiFi Shield         | Cloud connectivity for Blynk |
| Soil Moisture Sensor        | Detects soil water content |
| LDR (Light Dependent Resistor) | Measures ambient light |
| DHT11 Temperature Sensor    | Optional, for future temperature-based irrigation |
| Relay Module (Active LOW)   | Controls water pump |
| 16x2 I2C LCD               | Displays real-time readings for offline monitoring |
| LED and Buzzer              | Alerts for dry soil or low light conditions |

**Notes:**
- The system supports a **hybrid online-offline design**.
- Future versions may integrate additional sensors like rain detection and multi-zone soil probes.

---

## Software Architecture

**Arduino (Offline + Core Control)**
1. Reads soil moisture, light, and temperature.
2. Determines pump state using threshold logic:
   - `drySoil`: soil > threshold → Pump ON
   - `noSoil`: soil < min or > max → Pump OFF, alert
3. Updates the LCD with current sensor readings and pump status.
4. Sends sensor data via UART to ESP8266.

**ESP8266 (Online Cloud Communication)**
1. Connects to WiFi using SSID and password.
2. Authenticates with Blynk Cloud via **token-based authentication**.
3. Uploads sensor readings to virtual pins in the Blynk app.
4. Receives remote pump commands via virtual pin V2.
5. Controls the pump relay based on cloud commands while preserving local control logic.

---

## Networking Protocol

- **Communication Arduino ↔ ESP8266:** UART (SoftwareSerial) at 9600 baud.
- **Cloud Communication:** TCP/IP over WiFi handled by ESP8266 shield.
- **Blynk Protocol:** Token-based authentication ensuring encrypted cloud commands and data uploads.
- **Data Flow:** Arduino sends sensor values → ESP8266 → Blynk Cloud → Mobile App; reverse for pump commands.

---

## Security Implementation

1. **Token-Based Authentication:** Each device has a unique Blynk Auth Token.
2. **Encrypted Cloud Communication:** Prevents unauthorized access.
3. **Fail-Safe Offline Operation:** Local relay logic ensures that irrigation continues correctly even if the cloud connection is lost.

---

## Installation & Step-By-Step Setup

### Hardware Setup
1. Connect **soil sensor** to `A1` on Arduino.
2. Connect **LDR** to `A0`.
3. Connect **DHT11** to `A2`.
4. Connect **relay module** to pin `9` (Active LOW).
5. Connect **LED** to pin `7` and **buzzer** to pin `8`.
6. Connect **16x2 I2C LCD** to SDA/SCL pins (`A4`, `A5` on Uno).
7. Stack **ESP8266 WiFi shield** on top of Arduino or connect via SoftwareSerial pins 2 (RX) and 3 (TX).

### Software Setup
1. Install Arduino IDE and required libraries:
   - `LiquidCrystal_I2C`
   - `DHT`
   - `SoftwareSerial`
   - `ESP8266_Lib`
   - `Blynk`
2. Update WiFi credentials in the code:
   ```cpp
   char ssid[] = "Your_SSID";
   char pass[] = "Your_Password";'''
3. Update Blynk Auth Token:

  #define BLYNK_AUTH_TOKEN "Your_Blynk_Token"


Upload Arduino offline code to Arduino.

Upload ESP8266 Blynk code to Arduino (via shield).

Open Blynk mobile app, configure virtual pins:

V2 → Pump Switch

V5 → Soil Moisture

V6 → Light Intensity

Power the system, monitor LCD, and verify Blynk cloud readings.
