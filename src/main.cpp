#include <Arduino.h>
#include <DHT.h>
#include <SoftwareSerial.h>

// ============================================================
// HARDWARE PIN DEFINITIONS
// ============================================================
#define PIN_DHT         4       // Digital Pin for DHT Sensor
#define PIN_SOIL        A0      // Analog Pin for Capacitive Sensor
#define PIN_RX_ESP      2       // Arduino Pin 2 -> ESP8266 TX
#define PIN_TX_ESP      3       // Arduino Pin 3 -> ESP8266 RX

// ============================================================
// CONSTANTS & OBJECTS
// ============================================================
#define DHTTYPE DHT11           // Change to DHT22 if your friend has the white one
DHT dht(PIN_DHT, DHTTYPE);

SoftwareSerial espSerial(PIN_RX_ESP, PIN_TX_ESP); // The bridge to Wi-Fi

// Timing variables (Non-blocking delay)
unsigned long previousMillis = 0;
const long interval = 2000;     // Read sensors every 2 seconds

void setup() {
    // 1. Start the Console Debugging (USB to PC)
    Serial.begin(9600);
    Serial.println(F(">>> SYSTEM BOOT: Soil Telemetry Node <<<"));

    // 2. Start the ESP8266 Link
    espSerial.begin(9600); // Standard baud rate for Hayes commands
    Serial.println(F("[INIT] ESP8266 Serial Link Established"));

    // 3. Start Sensors
    dht.begin();
    pinMode(PIN_SOIL, INPUT);
    Serial.println(F("[INIT] Sensors Primed. Waiting for stable readings..."));
}

void loop() {
    unsigned long currentMillis = millis();

    // Run this logic block only every 'interval' (2 seconds)
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        // --- STEP 1: READ DHT (Air) ---
        float humidity = dht.readHumidity();
        float tempC = dht.readTemperature();

        // Check if DHT is physically connected or fried
        if (isnan(humidity) || isnan(tempC)) {
            Serial.println(F("[ERROR] DHT Sensor not responding! Check wiring on Pin 4."));
        }

        // --- STEP 2: READ SOIL (Ground) ---
        // This is RAW data (0-1023). 
        // 1023 is usually "Bone Dry" (in air). 
        // Lower numbers mean "Wet" (more conductivity).
        int soilRaw = analogRead(PIN_SOIL);

        // --- STEP 3: OUTPUT DIAGNOSTICS ---
        Serial.print(F("Time: "));
        Serial.print(currentMillis / 1000);
        Serial.print(F("s | Temp: "));
        Serial.print(tempC);
        Serial.print(F("C | Hum: "));
        Serial.print(humidity);
        Serial.print(F("% | Soil Raw: "));
        Serial.println(soilRaw);

        // Pass data to ESP (Just echoing for now to test the link)
        // We aren't sending web requests yet, just testing the wire.
        espSerial.print("RAW_DATA:");
        espSerial.println(soilRaw);
    }

    // --- STEP 4: ESP LISTENER ---
    // If the ESP says anything (like "Wifi Connected"), print it to our console.
    if (espSerial.available()) {
        Serial.write(espSerial.read());
    }
}