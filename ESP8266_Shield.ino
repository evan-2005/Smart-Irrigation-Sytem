/************************************************************
 * SMART IRRIGATION SYSTEM – ONLINE CONTROLLER (ESP8266)
 *
 * Role:
 * - Provides WiFi connectivity for the Arduino-based system
 * - Uploads sensor data to the Blynk cloud platform
 * - Receives remote pump control commands from mobile app
 *
 * Architecture Note:
 * - Arduino handles real-time sensing and offline automation
 * - ESP8266 shield handles all TCP/IP, WiFi, and cloud tasks
 * - Communication between Arduino and ESP8266 uses UART
 *
 * Security Model:
 * - Uses Blynk token-based authentication
 * - All cloud communication is authenticated and encrypted
 ************************************************************/

// ===================== BLYNK TEMPLATE CONFIG =====================
// Identifies the project and device inside Blynk Cloud
#define BLYNK_TEMPLATE_ID   "TMPL6npShQkeT"
#define BLYNK_TEMPLATE_NAME "smart irrigation system"

// Unique authentication token issued by Blynk Cloud
// Used for token-based device authentication
#define BLYNK_AUTH_TOKEN    "15tpBNQafDXnnaaOt2WYafvegNn1GSSD"

// Enables debug messages over USB Serial Monitor
#define BLYNK_PRINT Serial

// ESP8266 default baud rate for shield firmware
#define ESP8266_BAUD 9600

// ===================== REQUIRED LIBRARIES =====================
#include <SoftwareSerial.h>                 // UART communication
#include <ESP8266_Lib.h>                    // ESP8266 AT command wrapper
#include <BlynkSimpleShieldEsp8266.h>       // Blynk over ESP8266 shield

// ===================== HARDWARE PIN DEFINITIONS =====================
// Relay connected to Arduino digital pin
// LOW-trigger relay means LOW = pump ON
#define RELAY_PIN 9      

// Analog sensor inputs (shared with Arduino offline logic)
#define SOIL_PIN  A1     
#define LDR_PIN   A0     

// ===================== BLYNK TIMER =====================
// Used for non-blocking periodic tasks
BlynkTimer timer;

// ===================== WIFI & ESP8266 CONFIG =====================
// WiFi credentials (used by ESP8266 to connect to router)
char ssid[] = "SunwayEdu";
char pass[] = "";

// Software UART connection to ESP8266 shield
// RX, TX pins must match Arduino wiring
SoftwareSerial EspSerial(2, 3);

// Create ESP8266 object for Blynk library
ESP8266 wifi(&EspSerial);

// ================================================================
// BLYNK VIRTUAL PIN HANDLER – REMOTE PUMP CONTROL
// ================================================================
/*
 * Virtual Pin V2:
 * - Linked to a Switch widget in Blynk app
 * - Allows user to manually override pump state remotely
 *
 * Security:
 * - Command accepted only after successful token authentication
 */
BLYNK_WRITE(V2) {
  int pumpState = param.asInt();   // Read value from app (0 or 1)

  if (pumpState == 1) {
    digitalWrite(RELAY_PIN, LOW);    // Pump ON (active LOW relay)
  } else {
    digitalWrite(RELAY_PIN, HIGH);   // Pump OFF
  }
}

// ================================================================
// SENSOR DATA UPLOAD TASK
// ================================================================
/*
 * Periodic function executed by BlynkTimer
 * - Reads local sensor values from Arduino analog pins
 * - Uploads data to Blynk cloud using virtual pins
 *
 * Virtual Pins:
 * - V5 → Soil moisture
 * - V6 → Light intensity
 */
void sendSensors() {
  int soilValue  = analogRead(SOIL_PIN);
  int lightValue = analogRead(LDR_PIN);

  // Send sensor readings to Blynk dashboard
  Blynk.virtualWrite(V5, soilValue);
  Blynk.virtualWrite(V6, lightValue);
}

// ================================================================
// SETUP FUNCTION
// ================================================================
void setup() {
  // USB serial for debugging and logging
  Serial.begin(9600);

  // Initialize UART connection to ESP8266 shield
  EspSerial.begin(ESP8266_BAUD);
  delay(1000); // Allow ESP8266 to boot

  // Configure relay output
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // Pump OFF at startup (fail-safe)

  /*
   * Blynk.begin():
   * - Connects ESP8266 to WiFi network
   * - Authenticates device using Blynk Auth Token
   * - Establishes secure session with Blynk Cloud
   */
  Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass);

  // Schedule periodic sensor upload every 1 second
  timer.setInterval(1000L, sendSensors);
}

// ================================================================
// MAIN LOOP
// ================================================================
void loop() {
  /*
   * Blynk.run():
   * - Handles incoming cloud commands
   * - Maintains authenticated connection
   */
  Blynk.run();

  /*
   * timer.run():
   * - Executes scheduled non-blocking tasks
   */
  timer.run();
}
