/************************************************************
 * AQUASENSE - SMART IRRIGATION SYSTEM - OFFLINE CONTROLLER (ARDUINO)
 *
 * Role:
 * - Acts as the offline, real-time control unit
 * - Reads soil moisture, light intensity, and temperature
 * - Controls pump, LED, and buzzer locally (no internet needed)
 * - Sends sensor data to ESP8266 WiFi Shield for cloud upload
 * - Accepts pump override commands from cloud (Blynk)
 *
 * Design Rationale:
 * - Offline automation ensures reliability during WiFi outages
 * - ESP8266 handles networking to reduce Arduino RAM usage
 ************************************************************/

#include <Wire.h>                  // I2C communication
#include <LiquidCrystal_I2C.h>     // I2C LCD library
#include "DHT.h"                   // DHT temperature sensor
#include <SoftwareSerial.h>        // Serial communication with ESP8266

// ===================== PIN DEFINITIONS =====================
// Analog sensor inputs
#define LDR_PIN     A0              // Light Dependent Resistor
#define SOIL_PIN    A1              // Soil moisture sensor

// Digital outputs
#define LED_PIN     7               // Status LED
#define BUZZER_PIN  8               // Audible alert
#define RELAY_PIN   9               // Pump relay (Active LOW)

// DHT sensor configuration
#define DHT_PIN     A2              // DHT11 data pin
#define DHT_TYPE    DHT11

// ===================== THRESHOLD VALUES =====================
// Light threshold (lower value = darker environment)
int lightThreshold = 30;

// Soil moisture thresholds
int soilThreshold  = 700;           // Above this → soil is dry
int minSoilValue   = 50;            // Sensor disconnected / invalid
int maxSoilValue   = 1020;          // No soil / probe in air

// ===================== LCD CONFIGURATION =====================
LiquidCrystal_I2C lcd(0x27, 16, 2); // 16x2 I2C LCD

// ===================== DHT SENSOR OBJECT =====================
DHT dht(DHT_PIN, DHT_TYPE);

// ===================== ESP8266 SERIAL LINK =====================
// UART communication between Arduino and WiFi Shield
SoftwareSerial espSerial(2, 3);     // RX, TX

// ===================== UTILITY FUNCTION =====================
/*
 * Reads an analog sensor multiple times and returns the average.
 * Purpose:
 * - Reduces noise and unstable readings from analog sensors.
 */
int readAvg(int pin) {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(pin);
    delay(5);
  }
  return sum / 10;
}

// ===================== SETUP =====================
void setup() {
  Serial.begin(9600);               // USB serial for debugging
  espSerial.begin(9600);            // ESP8266 communication

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Configure output pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  // Ensure pump is OFF at startup
  digitalWrite(RELAY_PIN, HIGH);

  // Initialize temperature sensor
  dht.begin();

  // Startup message
  lcd.setCursor(0, 0);
  lcd.print("Smart Garden");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);
  lcd.clear();
}

// ===================== MAIN LOOP =====================
void loop() {

  // --------- SENSOR ACQUISITION ---------
  int ldrValue  = readAvg(LDR_PIN);     // Light intensity
  int soilValue = readAvg(SOIL_PIN);    // Soil moisture
  float temperatureC = dht.readTemperature();

  // Handle DHT read failure gracefully
  if (isnan(temperatureC)) temperatureC = 0;

  // --------- SOIL STATE CLASSIFICATION ---------
  bool noSoil  = soilValue <= minSoilValue || soilValue >= maxSoilValue;
  bool drySoil = soilValue > soilThreshold && !noSoil;

  // --------- LED STATUS INDICATOR ---------
  // LED ON if:
  // - Environment is dark OR
  // - Soil is dry
  digitalWrite(LED_PIN, (ldrValue < lightThreshold || drySoil) ? HIGH : LOW);

  // --------- PUMP & BUZZER CONTROL (OFFLINE LOGIC) ---------
  if (!noSoil && drySoil) {
    digitalWrite(RELAY_PIN, LOW);        // Pump ON
    tone(BUZZER_PIN, 1200, 150);         // Short alert tone
  } else {
    digitalWrite(RELAY_PIN, HIGH);       // Pump OFF
    noTone(BUZZER_PIN);
  }

  // --------- LCD DISPLAY (OFFLINE UI) ---------
  lcd.setCursor(0, 0);
  lcd.print("Light:");
  lcd.print(ldrValue);
  lcd.print("   ");

  lcd.setCursor(0, 1);
  lcd.print("Soil:");
  lcd.print(soilValue);
  lcd.print(" ");

  if (noSoil)       lcd.print("No Soil ");
  else if (drySoil) lcd.print("Dry     ");
  else              lcd.print("Wet     ");

  // --------- SEND SENSOR DATA TO ESP8266 ---------
  // Data forwarded to Blynk cloud by WiFi shield
  espSerial.print("LDR:");
  espSerial.print(ldrValue);
  espSerial.print(",SOIL:");
  espSerial.print(soilValue);
  espSerial.print(",TEMP:");
  espSerial.println(temperatureC, 1);

  // --------- RECEIVE CLOUD COMMANDS ---------
  // Allows Blynk app to override pump state
  if (espSerial.available()) {
    char cmd = espSerial.read();
    if (cmd == '1') digitalWrite(RELAY_PIN, LOW);   // Pump ON
    if (cmd == '0') digitalWrite(RELAY_PIN, HIGH);  // Pump OFF
  }

  delay(500); // Control loop timing
}
