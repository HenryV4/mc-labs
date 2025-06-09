#ifndef WIFI_H
#define WIFI_H

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to WiFi.");
  } else {
    Serial.println("WiFi connected!");
    Serial.print("Access the site at: http://");
    Serial.println(WiFi.localIP());
  }
}

#endif
