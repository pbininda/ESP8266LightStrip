#include <stdint.h>
#include <WiFiServer.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "settings.h"
#include "WiFiServe.h"
#include "OTA.h"
#include "HTTP.h"

static bool wiFiSetupDone = false;
static WiFiManager wifiManager;

String wiFiMac;

void startWiFiPortal() {
  Serial.print("Resetting WIFI");
  WiFi.disconnect(false, true);
  ESP.restart();
}

void initWiFi() {
  // Connect to WiFi network
  static uint8_t MAC_STA[] = {0,0,0,0,0,0};
  Serial.print("MAC[STA]");
  uint8_t *MAC  = WiFi.macAddress(MAC_STA);                   //get MAC address of STA interface // NOLINT(cppcoreguidelines-init-variables)

  wiFiMac = "";
  for (uint8_t i = 0; i < static_cast<uint8_t>(sizeof(MAC)+2); ++i) {
    Serial.print(":");
    Serial.print(MAC[i],HEX);
    wiFiMac = wiFiMac + String(MAC[i], HEX);
  }
  wifiManager.autoConnect(SYSTEM_NAME);
  // WiFi.setSleepMode(WIFI_NONE_SLEEP);
}

void handleWiFi(Led **leds, Effects **effects) {
  if (!wiFiSetupDone) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println(SYSTEM_NAME);
      Serial.println("WiFi connected");
//      wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
      Serial.println("initializing server\r\n");
      initServer(strip_settings, strip_states, leds, effects);
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
