#ifndef FAN_CONTROL_H
#define FAN_CONTROL_H

#include <Arduino.h>
#include <ESP8266HTTPClient.h>

// Ці змінні існують у main.ino
extern const int pwmPin;
extern int currentPWM;
extern const char* laptopIP;
extern float currentTemp;
extern int manualPWMPercent;

void setupPWM() {
  pinMode(pwmPin, OUTPUT);
  analogWriteFreq(20000);
  analogWriteRange(1023);
  analogWrite(pwmPin, 0);
}

int calculateFanPWM(float temp) {
  if (temp < 55.0) return 0;
  if (temp >= 90.0) return 1023;

  float normalized = (temp - 55.0) / 35.0;
  float curved = pow(normalized, 2.0);
  return int(curved * 307) + 716;
}

void logToLaptop(String mode, int pwm, float temp) {
  HTTPClient http;
  WiFiClient client;
  String url = "http://" + String(laptopIP) + ":5000/log?mode=" + mode + "&pwm=" + pwm + "&temp=" + String(temp, 1);
  
  http.setTimeout(1000); // максимум 1 секунда!
  if (http.begin(client, url)) {
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.printf("Log sent: %s (%d)\n", url.c_str(), httpCode);
    } else {
      Serial.printf("HTTP log error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("HTTP log connection failed");
  }
}


void updateFanSpeed(float temp) {
  int targetPWM = calculateFanPWM(temp);

  if (targetPWM == 0) {
    analogWrite(pwmPin, 0);
    currentPWM = 0;
    Serial.println("Fan OFF");
    return;
  }

  if (currentPWM < 720 && targetPWM > 720) {
    Serial.println("Spin-up boost to 100%");
    analogWrite(pwmPin, 1023);
    unsigned long start = millis();
    while (millis() - start < 1000) yield();
  }

  analogWrite(pwmPin, targetPWM);
  currentPWM = targetPWM;
  Serial.printf("Fan PWM set to %d (temp %.1f°C)\n", targetPWM, temp);
}

#endif
