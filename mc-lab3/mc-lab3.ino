#include "wifi_setup.h"
#include "fan_control.h"
#include "temperature_request.h"
#include "web_interface.h"
#include "oled_display.h"

const int pwmPin = 5;
int currentPWM = 0;
float currentTemp = 0.0;
unsigned long lastRequest = 0;
const unsigned long requestInterval = 10000;
const char* laptopIP = "";

FanMode currentMode = AUTO;
int manualPWMPercent = 0;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  setupPWM();
  connectWiFi();
  setupWebServer();
  setupWebInterfaceRoutes();
  setupOLED();
}

void loop() {
  server.handleClient();
  updateWebInterface();

  unsigned long now = millis();
  if (now - lastRequest >= requestInterval) {
    getTempFromLaptop();
    updateOLED();
    lastRequest = now;
  }

}
