/*
  DoinoCoin_Esp_Master.ino
  Created on: 10 05 2021

  Original Author: 
  Luiz H. Cassettari - https://github.com/ricaun/DuinoCoinI2C
  
  Modified & Tweaked By:  Maker Vinod (@maker.vinod)
  
*/
#if ESP8266
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#endif
#if ESP32
#include <HTTPClient.h>
#endif

#include <ArduinoJson.h>

const char * urlPool = "https://server.duinocoin.com/getPool";

void UpdateHostPort(String input)
{
  DynamicJsonDocument doc(256);
  deserializeJson(doc, input);

  const char* name = doc["name"];
  const char* ip = doc["ip"];
  int port = doc["port"];

  Serial.println("[ ]Update " + String(name) + " " + String(ip) + " " + String(port));
  SetHostPort(String(ip), port);
}

void UpdatePool()
{
  String input = httpGetString(urlPool);
  if (input == "") return;
  UpdateHostPort(input);
}

String httpGetString(String URL)
{
  String payload = "";
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  if (http.begin(client, URL))
  {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK)
    {
      payload = http.getString();
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      ESP.restart();
    }
    http.end();
  }
  return payload;
}
