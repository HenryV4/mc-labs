#include <ESP8266WiFi.h>
#include "wifi.h"
#include "oled_display.h"
#include "mqtt_client.h"

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  initDisplay();
  setupMQTT();
}

void loop() {
  handleMQTT();
}
