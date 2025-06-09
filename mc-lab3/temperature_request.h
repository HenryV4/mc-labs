#ifndef TEMP_REQUEST_H
#define TEMP_REQUEST_H

#include <Arduino.h>    
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include "web_interface.h"

extern float currentTemp;
extern unsigned long lastRequest;
extern const unsigned long requestInterval;
extern const char* laptopIP;

extern int currentPWM;
extern void updateFanSpeed(float temp);
extern void logToLaptop(String mode, int pwm, float temp);

extern FanMode currentMode;
extern int manualPWMPercent;

void getTempFromLaptop() {
  static float lastTemp = -100.0;

  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClient client;
  HTTPClient http;
  String url = String("http://") + laptopIP + ":5000/temp";

  http.setTimeout(1000);
  http.useHTTP10(true);
  if (http.begin(client, url)) {
    int httpCode = http.GET();

    if (httpCode == 200) {
      String payload = http.getString();
      currentTemp = payload.toFloat();

      // --- Вентилятор міняємо лише при зміні температури ---
      if (abs(currentTemp - lastTemp) >= 2.0) {
        lastTemp = currentTemp;
        if (currentMode == AUTO)
          updateFanSpeed(currentTemp);
      }

      // --- Логуємо завжди ---
      logToLaptop(currentMode == MANUAL ? "manual" : "auto", currentPWM, currentTemp);

      // --- Додаємо точку в історію для графіка ---
      addTempToHistory(currentTemp);
    }

    http.end();
    client.stop();
  }
}




#endif
