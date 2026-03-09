#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

const char* ssid = "ESP32_LED_Controller";
const char* password = "12345678";

const int ledPin = 2;
String ledState;

AsyncWebServer server(80);

void createFiles() {
  Serial.println("Creating files...");
  
  File file = SPIFFS.open("/index.html", FILE_WRITE);
  if(!file) {
    Serial.println("Failed to create index.html");
    return;
  }
  
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" type="text/css" href="style.css">
  <title>ESP32 LED Controller</title>
</head>
<body>
  <div class="topnav">
    <h1>ESP32 LED Controller</h1>
  </div>
  <div class="content">
    <div class="card-grid">
      <div class="card">
        <p class="card-title">LED Control</p>
        <p>Click to turn LED on or off</p>
        <p>LED State: <strong>%STATE%</strong></p>
        <div class="button-container">
          <a href="/on"><button class="button-on">ON</button></a>
          <a href="/off"><button class="button-off">OFF</button></a>
        </div>
      </div>
    </div>
  </div>
</body>
</html>
)rawliteral";
  
  file.print(html);
  file.close();
  Serial.println("index.html created");
  
  file = SPIFFS.open("/style.css", FILE_WRITE);
  if(!file) {
    Serial.println("Failed to create style.css");
    return;
  }
  
  String css = R"rawliteral(
html {
  font-family: Helvetica;
  display: inline-block;
  margin: 0px auto;
  text-align: center;
}
h1{
  color: #0F3376;
  padding: 2vh;
}
p{
  font-size: 1.5rem;
}
.button {
  display: inline-block;
  background-color: #008CBA;
  border: none;
  border-radius: 4px;
  color: white;
  padding: 16px 40px;
  text-decoration: none;
  font-size: 30px;
  margin: 2px;
  cursor: pointer;
}
.button2 {
  background-color: #f44336;
}
)rawliteral";
  
  file.print(css);
  file.close();
  Serial.println("style.css created");
}

void listSPIFFSFiles() {
  Serial.println("\nFiles in SPIFFS:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  int count = 0;
  while(file) {
    Serial.print("  - ");
    Serial.print(file.name());
    Serial.print(" (");
    Serial.print(file.size());
    Serial.println(" bytes)");
    file = root.openNextFile();
    count++;
  }
  if(count == 0) {
    Serial.println("  No files found!");
  }
  Serial.println();
}

String processor(const String& var){
  if(var == "STATE"){
    ledState = digitalRead(ledPin) ? "ON" : "OFF";
    return ledState;
  }
  return String();
}
 
void setup(){
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\nESP32 Starting...");
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS Mount Failed!");
    return;
  }
  Serial.println("SPIFFS Mounted Successfully!");
  
  createFiles();
  
  listSPIFFSFiles();

  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started!");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Request received for /");
    if(SPIFFS.exists("/index.html")) {
      Serial.println("index.html found, sending...");
      request->send(SPIFFS, "/index.html", String(), false, processor);
    } else {
      Serial.println("index.html NOT FOUND!");
      request->send(404, "text/plain", "index.html not found");
    }
  });
  
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Request received for style.css");
    if(SPIFFS.exists("/style.css")) {
      Serial.println("style.css found, sending...");
      request->send(SPIFFS, "/style.css", "text/css");
    } else {
      Serial.println("style.css NOT FOUND!");
      request->send(404, "text/plain", "style.css not found");
    }
  });

  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Turning LED ON");
    digitalWrite(ledPin, HIGH);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Turning LED OFF");
    digitalWrite(ledPin, LOW);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    Serial.print("404 - Not Found: ");
    Serial.println(request->url());
    request->send(404, "text/plain", "Page not found");
  });

  server.begin();
  Serial.println("Server started!");
}
 
void loop(){
  
}