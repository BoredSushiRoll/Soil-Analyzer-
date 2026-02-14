#include <Arduino.h>
#include <DHT.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// ============================================================
// HARDWARE PINS
// ============================================================
#define PIN_DHT         4       // White Sensor (DHT22)
#define PIN_SOIL        A0      // Analog Soil Moisture Sensor
#define PIN_RX_ESP      2       // UART RX (From NodeMCU D2)
#define PIN_TX_ESP      3       // UART TX (To NodeMCU D1 via Resistor)

// ============================================================
// OBJECTS
// ============================================================
LiquidCrystal_I2C lcd(0x27, 16, 2);  
DHT dht(PIN_DHT, DHT22);
SoftwareSerial espSerial(PIN_RX_ESP, PIN_TX_ESP); 

// ============================================================
// HELPER: TRANSMISSION PROTOCOL
// ============================================================
void sendData(float t, float h, int s1) {
    // Simulating 3 plants to match system scalability.
    // s1 is the actual sensor reading; s2 and s3 are placeholders.
    int s2 = 0; 
    int s3 = 0;

    // Data format required by NodeMCU: <Temp, Hum, Soil1, Soil2, Soil3>
    espSerial.print("<");
    espSerial.print(t, 1);
    espSerial.print(",");
    espSerial.print(h, 0);
    espSerial.print(",");
    espSerial.print(s1); // Plant 1 (Active Sensor)
    espSerial.print(",");
    espSerial.print(s2); // Plant 2 (Reserved)
    espSerial.print(",");
    espSerial.print(s3); // Plant 3 (Reserved)
    espSerial.println(">");
}

void setup() {
    Serial.begin(9600);     
    espSerial.begin(9600);  
    dht.begin(); 
    pinMode(PIN_SOIL, INPUT_PULLUP); 

    lcd.init();      
    lcd.backlight(); 
    
    lcd.setCursor(0, 0);
    lcd.print(F("SYS BOOT ON"));
    lcd.setCursor(0, 1);
    lcd.print(F("Initializing..."));
    delay(2000);
}

void loop() {
    // --- 1. DATA ACQUISITION ---
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    
    // Process Analog Soil Moisture Data
    int soilRaw = analogRead(PIN_SOIL);
    int soilPercent = map(soilRaw, 1023, 200, 0, 100); 
    soilPercent = constrain(soilPercent, 0, 100);

    // Sensor fail-safe
    if (isnan(t) || isnan(h)) { t = 0.0; h = 0.0; }

    // --- 2. LOCAL DISPLAY (LCD) ---
    lcd.clear();
    // Row 0: Air Parameters
    lcd.setCursor(0, 0); 
    lcd.print(F("A:")); lcd.print((int)t); lcd.print(F("C Hum:")); lcd.print((int)h); lcd.print(F("%"));

    // Row 1: Soil Status and TX heartbeat
    lcd.setCursor(0, 1);
    lcd.print(F("Soil 1:")); lcd.print(soilPercent); lcd.print(F("% >> TX"));

    // --- 3. DATA TRANSMISSION (TO NODE) ---
    sendData(t, h, soilPercent);

    // --- 4. SYSTEM DEBUG (TO PC) ---
    Serial.print("Sending Packet: <");
    Serial.print(t, 1); Serial.print(",");
    Serial.print(h, 0); Serial.print(",");
    Serial.print(soilPercent); Serial.println(",0,0>");

    delay(2000); // 2-second cycle
}