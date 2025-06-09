#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "";
const char* password = "";

extern ESP8266WebServer server;

void handlePing() {
  server.send(200, "text/plain", "ESP is here");
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("ESP IP: ");
  Serial.println(WiFi.localIP());
}

void setupWebServer() {
  server.on("/ping", handlePing);
  server.begin();
  Serial.println("HTTP server started");
}

#endif
