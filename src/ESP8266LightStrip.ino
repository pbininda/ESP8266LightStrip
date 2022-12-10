#include "WiFiServe.h"
#include "OTA.h"
#include "HTTP.h"
#include "LED.h"
#include "state.h"
#include "persistence.h"
#include "effects.h"

const bool DEBUG_TIMING = 0;

uint16_t briLevels[] = {4, 16, 64, 256};
uint8_t NUM_BRILEVELS = (sizeof briLevels) / (sizeof (uint8_t));

void setLeds() {
  switch (settings.mode) {
    default:
    case 0:
      {
        setLedsFixed(ledColor(state.dynR, state.dynG, state.dynB, settings.bri2));
      }
      break;
    case 1:
      {
        setLedsZylon();
      }
      break;
    case 2:
      {
        setLedsRainbowCycle();
      }
      break;
  }
  sendLeds();
}

void loop() {
  static const int tickResolution = 1000;
  handleWiFi();
  handleServer();
  updateState(NUM_LEDS);
  setLeds();
  handleOta();
  if (getLastLedChangeDelta() > 1000) {
    if (DEBUG_TIMING) {
      Serial.println("no color changes => sleeping");
    }
    wiFiGoToSleep(200);
  }
  if (DEBUG_TIMING && state.tick % tickResolution == 0) {
    static time_t lastTickTime = 0;
    Serial.print("tick time: ");
    Serial.print((state.now - lastTickTime) / (tickResolution * 1.0));
    Serial.println("ms");
    lastTickTime = state.now;
  }
  state.tick++;
  // delay(10);
  // yield();
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("initializing LED Strip for Bathroom\r\n");
  readSettings();
  initState();
  initLeds();
  setLeds();
  Serial.println("initializing WIFI\r\n");
  initWiFi();
}
