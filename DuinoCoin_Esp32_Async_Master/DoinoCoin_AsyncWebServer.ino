/*
  DoinoCoin_Esp_Master.ino
  Created on: 10 05 2021

  Original Author: 
  Luiz H. Cassettari - https://github.com/ricaun/DuinoCoinI2C
  
  Modified & Tweaked By:  Maker Vinod (@maker.vinod)
  
*/
#if ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <FS.h>
#endif
#if ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#endif

#include <SPIFFSEditor.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char* http_username = "admin";
const char* http_password = "admin";

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len);

void server_setup()
{
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

#ifdef ESP8266
  SPIFFS.begin();
  server.addHandler(new SPIFFSEditor(http_username, http_password));
#endif
#ifdef ESP32
  SPIFFS.begin(true);
  server.addHandler(new SPIFFSEditor(SPIFFS, http_username, http_password));
#endif

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.on("/set", HTTP_GET, [](AsyncWebServerRequest * request) {
    String ret = "";
    if(request->hasArg("host"))
    {
      String host = request->arg("host");
      ret += SetHost(host);
    }
    ret += " ";
    if(request->hasArg("port"))
    {
      int port = request->arg("port").toInt();
      SetPort(port);
      ret += port;
    }
    request->send(200, "text/plain", ret);
  });

  server.on("/clients", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", clients_show());
  });


  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.htm");

  server.begin();
}

void ws_sendAll(String payload)
{
  if (ws.count() > 0)
  {
    ws.textAll(payload);
  }
}

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("Hello Client %u :)", client->id());
    client->ping();
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id());
  } else if (type == WS_EVT_ERROR) {
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  } else if (type == WS_EVT_PONG) {
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char*)data : "");
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    String msg = "";
    if (info->final && info->index == 0 && info->len == len) {
      //the whole message is in a single frame and we got all of it's data
      Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

      if (info->opcode == WS_TEXT) {
        for (size_t i = 0; i < info->len; i++) {
          msg += (char) data[i];
        }
      } else {
        char buff[3];
        for (size_t i = 0; i < info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          msg += buff;
        }
      }

      wire_SendAll(msg.c_str());
      
      Serial.printf("%s\n", msg.c_str());

      if (info->opcode == WS_TEXT)
        client->text("I got your text message");
      else
        client->binary("I got your binary message");
    } 
  }
}
