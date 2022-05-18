/*
  DoinoCoin_Esp_Master.ino
  Created on: 10 05 2021

  Original Author: 
  Luiz H. Cassettari - https://github.com/ricaun/DuinoCoinI2C
  
  Modified & Tweaked By:  Maker Vinod (@maker.vinod)
  
*/

void wire_setup();
void wire_readAll();
boolean wire_exists(byte address);
void wire_sendJob(byte address, String lastblockhash, String newblockhash, int difficulty);
void Wire_sendln(byte address, String message);
void Wire_send(byte address, String message);
String wire_readLine(int address);
boolean wire_runEvery(unsigned long interval);

const char* ssid          = "{YOUR-WIFI-SSID}";              // Change this to your WiFi SSID
const char* password      = "{YOUR-WIFI-PASSWORD}";              // Change this to your WiFi password
const char* ducouser      = "{DUINO-COIN-USERNAME}";        // Change this to your Duino-Coin username
const char* rigIdentifier = "MINER-RIG-1";  // Change this if you want a custom miner name

#if ESP8266
#include <ESP8266WiFi.h> // Include WiFi library
#include <ESP8266mDNS.h> // OTA libraries
#include <WiFiUdp.h>
#endif
#if ESP32
#include <WiFi.h>
#endif

#include <ArduinoOTA.h>
#include <StreamString.h>

#define BLINK_SHARE_FOUND    1
#define BLINK_SETUP_COMPLETE 2
#define BLINK_CLIENT_CONNECT 3
#define BLINK_RESET_DEVICE   5

#if ESP8266
#define LED_BUILTIN 2
#define MINER "AVR I²C v3.0"
#define JOB "AVR"
#endif

#if ESP32
#define LED_BUILTIN 2
#define MINER "AVR I²C v3.0"
#define JOB "AVR"
#endif


void SetupWifi() {
  Serial.println("Connecting to: " + String(ssid));
  WiFi.mode(WIFI_STA); // Setup ESP in client mode

  if (ssid == "")
    WiFi.begin();
  else
    WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
  Serial.println("Local IP address: " + WiFi.localIP().toString());
}

void SetupOTA() {
  ArduinoOTA.onStart([]() { // Prepare OTA stuff
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

#if ESP8266
  char hostname[32];
  sprintf(hostname, "Miner-Async-%06x", ESP.getChipId());
  ArduinoOTA.setHostname(hostname);
#endif

#if ESP32
  char hostname[32];
  sprintf(hostname, "Miner32-Async-%06x", ESP.getEfuseMac());
  ArduinoOTA.setHostname(hostname);
#endif


  ArduinoOTA.begin();
}


void blink(uint8_t count, uint8_t pin = LED_BUILTIN) {
  uint8_t state = HIGH;
  for (int x = 0; x < (count << 1); ++x) {
    digitalWrite(pin, state ^= HIGH);
    delay(50);
  }
}

void RestartESP(String msg) {
  Serial.println(msg);
  Serial.println("Resetting ESP...");
  blink(BLINK_RESET_DEVICE);
#if ESP8266
  ESP.reset();
#endif
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  Serial.print("\nDuino-Coin ");
  Serial.println(MINER);
  //  oled_setup(); //Not using OLED
  wire_setup();
  delay(3000);
  SetupWifi();
  //  SetupOTA(); //Not using OTA
  //  oled_display(WiFi.localIP().toString() + "\n" + String(ESP.getFreeHeap()) + "\n" + clients_string());

  server_setup();
  UpdatePool();
  blink(BLINK_SETUP_COMPLETE);
  //  setupMultiCoreTasks();
}

void loop() {
  //  ArduinoOTA.handle(); //Not using OTA
  clients_loop();
  if (runEvery(10000))
  {
    Serial.print("[ ]");
    Serial.println("FreeRam: " + String(ESP.getFreeHeap()) + " " + clients_string());
    //    ws_sendAll("FreeRam: " + String(ESP.getFreeHeap()) + " - " + clients_string());
    //    oled_display(WiFi.localIP().toString() + "\n" + String(ESP.getFreeHeap()) + "\n" + clients_string());
  }
}

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
