#include <Arduino.h>
#include <DHT.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

// ============================================================
// HARDWARE PIN DEFINITIONS
// ============================================================
#define PIN_DHT         4       // White Sensor
#define PIN_SOIL        A0      // Direct Drive Prongs
#define PIN_RX_ESP      2       
#define PIN_TX_ESP      3       

// LCD CONFIGURATION
// Address is usually 0x27 or 0x3F.
LiquidCrystal_I2C lcd(0x27, 16, 2);  

// SENSORS
#define DHTTYPE DHT22           
DHT dht(PIN_DHT, DHTTYPE);

void setup() {
    Serial.begin(9600);
    
    // 1. Start Sensors
    dht.begin(); 
    pinMode(PIN_SOIL, INPUT_PULLUP); 

    // 2. Start LCD
    lcd.init();      // Initialize the LCD
    lcd.backlight(); // Turn on the light
    
    // Boot Screen
    lcd.setCursor(0, 0); // Top Left
    lcd.print(F("SYSTEM BOOT..."));
    lcd.setCursor(0, 1); // Bottom Left
    lcd.print(F("Sensors: OK"));
    
    delay(2000);
}

void loop() {
    // --- READ DATA ---
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    int soilRaw = analogRead(PIN_SOIL);
    
    // Drive Mapping
    int soilPercent = map(soilRaw, 1023, 200, 0, 100); 
    soilPercent = constrain(soilPercent, 0, 100);

    // --- DRAW TO LCD ---
    lcd.clear();
    
    // Line 1: Air Temp & Humidity
    lcd.setCursor(0, 0); 
    lcd.print(F("Air:"));
    lcd.print((int)t); // Cast to int to save space
    lcd.print(F("C Hum:"));
    lcd.print((int)h);
    lcd.print(F("%"));

    // Line 2: Soil Moisture
    lcd.setCursor(0, 1);
    lcd.print(F("Soil H2O: "));
    lcd.print(soilPercent);
    lcd.print(F("%"));

    // --- SERIAL DEBUG ---
    Serial.print(F("Air: ")); Serial.print(t, 1); 
    Serial.print(F(" | Soil Wetness: ")); Serial.println(soilPercent); 

    delay(3000);
}