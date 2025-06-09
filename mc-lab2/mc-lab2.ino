#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "btn.h"
#include "wifi.h"
#include "indexHTML.h"

#define LED1 14  // D5 (Red LED)
#define LED2 2   // D4 (Yellow LED)
#define LED3 13  // D7 (Green LED)
#define BUTTON_PIN 12

ESP8266WebServer server(80);
bool ownAlgorithmRunning = false;
int lastLED = 0;
bool uartSent = false;
int numLeds = 3;

// Байтове значення, яке відправляємо для "D" (0x44 = 'D' в ASCII)
const uint8_t COMMAND_D = 0x44;  

void setup() {
  // Зверніть увагу, що SERIAL_8N1 уже каже, що дані передаються 8-бітними словами
  Serial.begin(9600, SERIAL_8N1);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  turnOffLEDs();
  connectToWiFi();

  // Маршрути (Routes)
  server.on("/toggleOwn", HTTP_GET, []() {
    toggleOwnAlgorithm();
    server.send(200, "text/plain", ownAlgorithmRunning ? "ON" : "OFF");
  });

  server.on("/togglePartner", HTTP_GET, []() {
    // ВІДПРАВКА ЯК БАЙТ: використовуємо write замість print
    Serial.write(COMMAND_D); 
    uartSent = true;    
    server.send(200, "text/plain", "Sent byte 0x44 ('D') to Partner");
  });

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlPage);
  });

  server.begin();
}

void loop() {
  server.handleClient();
  handleUART();
  handleButton();

  if (ownAlgorithmRunning) {
    runLEDSequence();
  }
}

void handleUART() {
  if (Serial.available()) {
    // ЧИТАЄМО БАЙТ ВМІСТО char
    uint8_t command = Serial.read();
    if (command == COMMAND_D) {
      // Якщо ми отримали 0x44 ('D')
      if (!uartSent) {
        toggleOwnAlgorithm();
      }
      uartSent = false;
    }
  }
}

void handleButton() {
  if (detectDoubleClick()) {  
    toggleOwnAlgorithm();
  }
}

void toggleOwnAlgorithm() {
  ownAlgorithmRunning = !ownAlgorithmRunning;
}

void runLEDSequence() {
  static unsigned long lastMillis = 0;
  const int interval = 500;
  int ledPins[] = {LED1, LED3, LED2};

  if (millis() - lastMillis > interval) {
    lastMillis = millis();
    digitalWrite(ledPins[lastLED], LOW);
    lastLED = (lastLED + 1) % numLeds;
    digitalWrite(ledPins[lastLED], HIGH);
  }
}

void turnOffLEDs() {
  int ledPins[] = {LED1, LED3, LED2};
  for (int i = 0; i < numLeds; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}
