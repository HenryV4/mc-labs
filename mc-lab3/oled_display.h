#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "temperature_request.h"  // Щоб отримати currentTemp

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1  // Немає апаратного reset
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setupOLED() {
  Wire.begin(4, 12);  // SDA = GPIO4 (D2), SCL = GPIO12 (D6)

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED not found"));
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Initializing..."));
  display.display();
}


void updateOLED() {
  display.clearDisplay();

  // Температура
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print(F("Temp:"));

  display.setTextSize(2);
  display.setCursor(0, 12);

  if (currentTemp >= 90.0) {
    display.print(F("!!! "));
  }

  display.print(currentTemp, 1);
  display.print(F(" C"));

  // Режим
  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print(F("Mode: "));
  display.print(currentMode == MANUAL ? "Manual" : "Auto");

  // PWM з %
  display.setCursor(0, 52);
  display.print(F("PWM: "));
  display.print(currentPWM);

  int percentPWM = round((float)currentPWM / 1023.0 * 100);
  display.print(" (");
  display.print(percentPWM);
  display.print("%)");

  display.display();
}


#endif

