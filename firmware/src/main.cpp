#include <Arduino.h>
#include <WiFi.h>
#include "Application.h"
#include "config.h"

unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 10000; // 10 seconds
Application *application = nullptr;
bool wasConnected = false;

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting up");
  delay(1000);
  
  // WiFi.onEvent(WiFiEvent);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  
  // disable WiFi sleep mode
  WiFi.setSleep(WIFI_PS_NONE);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.println("");
  
  Serial.println("Creating microphone");
  application = new Application();
  application->begin();
  wasConnected = true;
}

void loop()
{
  static wl_status_t lastStatus = WL_CONNECTED;
  wl_status_t currentStatus = WiFi.status();
  
  if (currentStatus != lastStatus) {
    Serial.print("[WiFi] Status changed: ");
    Serial.println(currentStatus);
    lastStatus = currentStatus;
  }
  
  if (WiFi.status() != WL_CONNECTED)
  {
    if (wasConnected) {
      Serial.println("[WiFi] Connection lost - stopping application");
      if (application) {
        delete application;
        application = nullptr;
      }
      wasConnected = false;
    }
    
    unsigned long now = millis();
    if (now - lastReconnectAttempt > reconnectInterval)
    {
      Serial.println("[WiFi] Reconnecting...");
      WiFi.disconnect();
      WiFi.reconnect();
      lastReconnectAttempt = now;
    }
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
  else if (!wasConnected) {
    // WiFi just reconnected
    Serial.println("[WiFi] Reconnected - restarting application");
    Serial.println("IP address: ");
    Serial.print(WiFi.localIP());
    Serial.println("");
    
    if (application) {
      delete application;
    }
    application = new Application();
    application->begin();
    wasConnected = true;
  }
  
  vTaskDelay(pdMS_TO_TICKS(1000));
}
