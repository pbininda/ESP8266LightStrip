#include <stdint.h>
#include <WiFiServer.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "WiFiServe.h"
#include "OTA.h"
#include "HTTP.h"
#include "SSID_PASSWORD.h"

static const char* ssid = WIFI_SSID;
static const char* password = WIFI_PASSWORD;

static bool wiFiSetupDone = false;
static WiFiManager wifiManager;


void initWiFi() {
  // Connect to WiFi network
  static uint8_t  MAC_STA[]                = {0,0,0,0,0,0};
  Serial.print("MAC[STA]");
  uint8_t *MAC  = WiFi.macAddress(MAC_STA);                   //get MAC address of STA interface
  for (uint8_t i = 0; i < sizeof(MAC)+2; ++i){
    Serial.print(":");
    Serial.print(MAC[i],HEX);
  }
  wifiManager.autoConnect("^Generische Lampe");
  // WiFi.setSleepMode(WIFI_NONE_SLEEP);
}

void handleWiFi() {
  if (!wiFiSetupDone) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
//      wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
      Serial.println("initializing server\r\n");
      initServer();
      Serial.println("ready for commands\r\n");
      initOta();
      wiFiSetupDone = true;
    }
  }
}

void wiFiGoToSleep(uint32_t delayMs) {
//    wifi_set_sleep_type(LIGHT_SLEEP_T);
    delay(delayMs);
}
