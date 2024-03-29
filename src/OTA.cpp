#include <stdint.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "OTA.h"

static bool otaSetupDone = false;

void initOta() { // cppcheck-suppress unusedFunction
  String hostname("ESP32-OTA-"); // NOLINT(cppcoreguidelines-init-variables)
  hostname += String(ESP.getEfuseMac(), HEX);
  WiFi.hostname(hostname);
  Serial.println("Hostname: " + hostname);
  // Start OTA server.
  ArduinoOTA.setHostname((const char *)hostname.c_str());
  // ArduinoOTA.setPassword((const char *)"123");
  ArduinoOTA.onStart([]() {
    /*
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
    */
    Serial.println("Start updating");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  otaSetupDone = true;
  Serial.println("ready for OTA");
}

void handleOta() { // cppcheck-suppress unusedFunction
  if (otaSetupDone) {
    ArduinoOTA.handle();
  }
}
