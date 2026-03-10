#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"

const int pinSensor = 21;
const int pinRelay = 18;

volatile int pulsos = 0;
float factor_conversion = 7.5;
float caudal_Lmin = 0;
float volumen = 0;

unsigned long tiempoAnterior = 0;

const char* ssid = "ESP32_Valve_Controller";
const char* password = "12345678";

String valveState = "CLOSED";
bool valveOpen = false;

AsyncWebServer server(80);

void IRAM_ATTR contarPulsos() {
  pulsos++;
}

String processor(const String& var){
  if(var == "STATE"){
    return valveState;
  }
  if(var == "FLOW"){
    return String(caudal_Lmin, 2);
  }
  if(var == "VOLUME"){
    return String(volumen, 3);
  }
  return String();
}

void setup(){

  Serial.begin(115200);

  pinMode(pinSensor, INPUT);

  pinMode(pinRelay, INPUT);
  valveState = "CLOSED";
  valveOpen = false;

  attachInterrupt(digitalPinToInterrupt(pinSensor), contarPulsos, RISING);

  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS Mount Failed!");
    return;
  }

  WiFi.softAP(ssid, password);

  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/open", HTTP_GET, [](AsyncWebServerRequest *request){

    Serial.println("Opening valve");

    pinMode(pinRelay, OUTPUT);
    digitalWrite(pinRelay, HIGH);

    valveState = "OPEN";
    valveOpen = true;

    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/close", HTTP_GET, [](AsyncWebServerRequest *request){

    Serial.println("Closing valve");

    pinMode(pinRelay, INPUT);

    valveState = "CLOSED";
    valveOpen = false;

    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    volumen = 0;
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.begin();
}

void loop(){

  if (valveOpen && (millis() - tiempoAnterior >= 1000)) {

    detachInterrupt(digitalPinToInterrupt(pinSensor));

    float frecuencia = pulsos;

    caudal_Lmin = frecuencia / factor_conversion;

    volumen += caudal_Lmin / 60.0;

    Serial.print("Caudal: ");
    Serial.print(caudal_Lmin);
    Serial.println(" L/min");

    Serial.print("Volumen acumulado: ");
    Serial.print(volumen);
    Serial.println(" L");

    pulsos = 0;

    attachInterrupt(digitalPinToInterrupt(pinSensor), contarPulsos, RISING);

    tiempoAnterior = millis();
  }

  delay(10);
}