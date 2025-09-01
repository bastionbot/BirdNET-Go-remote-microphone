#include <Arduino.h>
#include <WiFi.h>
#include "Application.h"
#include "config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "time.h"

unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 10000; // 10 seconds
Application *application = nullptr;
bool wasConnected = false;

void wifiWatchdog( void *pvParameters );
void checkSleepWindow();

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

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  } else {
    Serial.println(&timeinfo, "Time synced: %H:%M:%S");
  }

  // Check immediately if we should be sleeping
  checkSleepWindow();

  Serial.println("Creating microphone");
  application = new Application();
  application->begin();
  wasConnected = true;
  xTaskCreate(wifiWatchdog, "WiFi Watchdog", 4096, NULL, 1, NULL);
}

void loop()
{
}

void checkSleepWindow()
{
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    int hour = timeinfo.tm_hour;

    if (hour >= sleep_hour || hour < wake_hour) {
      int sleepHours = (hour >= sleep_hour) ? (24 - hour + wake_hour) : (wake_hour - hour);
      uint64_t sleepSeconds = sleepHours * 3600ULL
                            - timeinfo.tm_min * 60ULL
                            - timeinfo.tm_sec;

      Serial.printf("Entering deep sleep for %llu seconds\n", sleepSeconds);

      esp_sleep_enable_timer_wakeup(sleepSeconds * 1000000ULL);
      esp_deep_sleep_start();
    }
  }
}

void wifiWatchdog( void *pvParameters )
{
  while(true) {
    static wl_status_t lastStatus = WL_CONNECTED;
    wl_status_t currentStatus = WiFi.status();

    if (currentStatus != lastStatus) {
      Serial.print("[WiFi] Status changed: ");
      Serial.println(currentStatus);
      Serial.flush();
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
      Serial.flush();

      if (application) {
        delete application;
      }
      application = new Application();
      application->begin();
      wasConnected = true;
    }

    checkSleepWindow();

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
