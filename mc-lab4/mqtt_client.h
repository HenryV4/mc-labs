#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>   // Secure клієнт для TLS
#include <PubSubClient.h>
#include "oled_display.h"

// Дані HiveMQ Cloud
const char* mqtt_server = "14edf8292a284130a4fe4542d2a7982b.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_topic = "esp8266/oled/text";
const char* mqtt_user = "HenryV4";
const char* mqtt_password = "Qwerty1212";

WiFiClientSecure espClient;   // Ось тут змінити WiFiClient на WiFiClientSecure
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
  showText(message);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setupMQTT() {
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void handleMQTT() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

#endif
