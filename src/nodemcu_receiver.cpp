#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SoftwareSerial.h>
#include <vector>
#include <LittleFS.h>

// ============================================================
// 1. NETWORK CONFIGURATION
// ============================================================
const char* ssid     = "YOUR_SSID_HERE";
const char* password = "YOUR_PASSWORD_HERE";

struct Plant {
  String name;
  int soilValue;
};

// Global Data State
std::vector<Plant> greenhouse;
float temp = 0.0, hum = 0.0;

ESP8266WebServer server(80);
SoftwareSerial link(D1, D2); // RX, TX Bridge to Arduino

// ============================================================
// 2. PERSISTENT STORAGE (LittleFS)
// ============================================================
void saveToFlash() {
  File f = LittleFS.open("/plants.csv", "w");
  if (f) {
    for (const auto& p : greenhouse) f.println(p.name);
    f.close();
  }
}

void loadFromFlash() {
  if (!LittleFS.exists("/plants.csv")) return;
  File f = LittleFS.open("/plants.csv", "r");
  if (f) {
    greenhouse.clear();
    while (f.available()) {
      String n = f.readStringUntil('\n');
      n.trim();
      if (n.length() > 0) greenhouse.push_back({n, 0});
    }
    f.close();
  }
}

// ============================================================
// 3. UI GENERATORS (HTML/CSS)
// ============================================================
String getDashboard() {
  String ptr = "<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr += "<meta http-equiv='refresh' content='2'><title>CONTROL PANEL</title>";
  ptr += "<style>";
  ptr += "body{font-family:Helvetica; background:#f4f4f4; text-align:center; padding:20px;}";
  ptr += ".card{background:white; padding:20px; margin:10px auto; border-radius:12px; max-width:400px; box-shadow:0 4px 10px rgba(0,0,0,0.1);}";
  ptr += ".air-header{font-size:26px; font-weight:bold; color:#444; margin-bottom:10px; border-bottom:2px solid #0275d8; display:inline-block; padding:0 10px;}";
  ptr += ".big-data{font-size:45px; font-weight:bold; color:#333; display:block; margin:5px 0;}";
  ptr += ".unit{font-size:20px; color:#888;}";
  ptr += "h3{font-size:32px; color:#333; margin:10px 0 5px 0;}"; 
  ptr += ".moisture-text{color:#0275d8; font-size:26px; font-weight:bold; margin-top:10px; display:block;}";
  ptr += ".btn{display:inline-block; padding:12px 24px; text-decoration:none; border-radius:8px; margin:10px; color:white; font-weight:bold;}";
  ptr += "</style></head><body><h1>GREENHOUSE MONITOR</h1>";
  
  // CENTERED AIR DATA STACK
  ptr += "<div class='card'>";
  ptr += "<div class='air-header'>Climate</div>";
  ptr += "<div>AIR TEMP</div><div class='big-data'>" + String(temp, 1) + "<span class='unit'>°C</span></div>";
  ptr += "<hr style='width:40%; border:0.5px solid #eee; margin:15px auto;'>";
  ptr += "<div>AIR HUMIDITY</div><div class='big-data'>" + String(hum, 0) + "<span class='unit'>%</span></div>";
  ptr += "</div>";

  // DYNAMIC PLANT ENTRIES
  for (size_t i = 0; i < greenhouse.size(); i++) {
    ptr += "<div class='card'>";
    ptr += "<h3>" + greenhouse[i].name + "</h3>";
    
    if (i == 0) {
      ptr += "<div class='moisture-text'>" + String(greenhouse[i].soilValue) + "% Plant Soil Moisture</div>";
    } else {
      ptr += "<div style='color:#888; font-size:24px; font-style:italic; margin-top:10px;'>Status: Disconnected</div>";
      ptr += "<div style='color:#bbb; font-size:14px;'>No hardware sensor (Slot " + String(i + 1) + ")</div>";
    }
    ptr += "</div>";
  }

  ptr += "<a href='/add' class='btn' style='background:#0275d8'>+ Add Plant</a>";
  ptr += "<a href='/manage' class='btn' style='background:#555'>Settings</a>";
  ptr += "</body></html>";
  return ptr;
}

String getManagePage() {
  String ptr = "<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr += "<style>body{font-family:Helvetica; background:#333; color:white; text-align:center; padding:20px;}";
  ptr += ".item{background:#444; padding:15px; margin:10px auto; max-width:450px; border-radius:10px; display:flex; justify-content:space-between; align-items:center;}";
  ptr += "input[type=text]{padding:8px; border-radius:5px; border:none; width:65%; font-size:16px;}";
  ptr += ".del{background:#ff4444; color:white; padding:8px 12px; text-decoration:none; border-radius:5px;}";
  ptr += ".save{background:#4CAF50; color:white; border:none; padding:8px 12px; border-radius:5px; cursor:pointer;}";
  ptr += ".wipe{background:#ff8c00; margin-top:40px; padding:15px; display:block; text-decoration:none; color:white; border-radius:10px; font-weight:bold;}";
  ptr += "</style></head><body><h1>Plant Management</h1>";

  for (size_t i = 0; i < greenhouse.size(); i++) {
    ptr += "<div class='item'>";
    ptr += "<form action='/rename' method='POST' style='margin:0; width:100%; display:flex; justify-content:space-around;'>";
    ptr += "<input type='hidden' name='id' value='" + String(i) + "'>";
    ptr += "<input type='text' name='newName' value='" + greenhouse[i].name + "'>";
    ptr += "<input type='submit' class='save' value='💾'>";
    if (i > 0) ptr += "<a href='/delete?id=" + String(i) + "' class='del'>🗑️</a>";
    ptr += "</form></div>";
  }
  ptr += "<a href='/' style='color:#aaa; display:block; margin-top:20px;'>Back to Dashboard</a>";
  ptr += "<a href='/wipe' class='wipe' onclick=\"return confirm('WARNING: Reset all plant entries?')\">TOTAL RESET</a>";
  ptr += "</body></html>";
  return ptr;
}

String getAddPage(String error = "") {
  String ptr = "<html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr += "<style>body{font-family:Helvetica; background:#333; color:white; text-align:center; padding-top:50px;} input{padding:12px; border-radius:8px; border:none; font-size:18px;}</style></head><body>";
  ptr += "<h1>Add New Entry</h1>";
  if(error != "") ptr += "<p style='color:#ff6b6b'>" + error + "</p>";
  ptr += "<form action='/create' method='POST'><input type='text' name='plantName' placeholder='Name...' autofocus><br>";
  ptr += "<input type='submit' style='background:#4CAF50; color:white; margin-top:20px; padding:12px 30px; border:none; border-radius:8px; cursor:pointer;' value='Confirm'></form></body></html>";
  return ptr;
}

// ============================================================
// 4. SERVER ROUTING & LOGIC
// ============================================================
void setup() {
  Serial.begin(9600); 
  link.begin(9600);
  
  if (LittleFS.begin()) loadFromFlash();
  if (greenhouse.size() == 0) greenhouse.push_back({"Primary Pot", 0});

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  server.on("/", []() { server.send(200, "text/html", getDashboard()); });
  server.on("/manage", []() { server.send(200, "text/html", getManagePage()); });
  server.on("/add", []() { server.send(200, "text/html", getAddPage()); });

  server.on("/rename", []() {
    int id = server.arg("id").toInt();
    String name = server.arg("newName"); name.trim();
    if (name.length() > 0 && id < greenhouse.size()) { greenhouse[id].name = name; saveToFlash(); }
    server.sendHeader("Location", "/manage"); server.send(303);
  });

  server.on("/delete", []() {
    int id = server.arg("id").toInt();
    if (id > 0 && id < greenhouse.size()) { greenhouse.erase(greenhouse.begin() + id); saveToFlash(); }
    server.sendHeader("Location", "/manage"); server.send(303);
  });

  server.on("/wipe", []() {
    if (greenhouse.size() > 1) { greenhouse.erase(greenhouse.begin() + 1, greenhouse.end()); saveToFlash(); }
    server.sendHeader("Location", "/"); server.send(303);
  });

  server.on("/create", []() {
    String n = server.arg("plantName"); n.trim();
    if (n.length() < 1) server.send(200, "text/html", getAddPage("Error: Please enter a valid name!"));
    else { greenhouse.push_back({n, 0}); saveToFlash(); server.sendHeader("Location", "/"); server.send(303); }
  });

  server.begin();
}

void loop() {
  server.handleClient();
  static String buffer = "";
  while (link.available()) {
    char c = link.read();
    if (c == '<') buffer = "<";
    else if (c == '>') {
      buffer += ">"; buffer.replace("<",""); buffer.replace(">","");
      int fC = buffer.indexOf(','); int sC = buffer.indexOf(',', fC + 1);
      temp = buffer.substring(0, fC).toFloat();
      hum = buffer.substring(fC + 1, sC).toFloat();
      if(greenhouse.size() > 0) {
        int tC = buffer.indexOf(',', sC + 1);
        greenhouse[0].soilValue = buffer.substring(sC + 1, tC).toInt();
      }
      buffer = "";
    } else buffer += c;
  }
}