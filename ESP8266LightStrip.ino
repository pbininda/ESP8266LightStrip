#include <ESP8266WiFi.h>
#include "OTA.h"
#include "HTTP.h"
#include "LED.h"
#include "state.h"
#include "persistence.h"

extern "C" {
  #include "user_interface.h"
}

#include "SSID_PASSWORD.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

const bool DEBUG_TIMING = 0;
bool wiFiSetupDone = false;

struct palette palette[] = {
  {255, 255, 255},
  {255, 200, 120},
  {255, 0, 0},
  {0, 255, 0},
  {0, 0, 255},
  {255, 200, 0},
  {0, 255, 200},
  {255, 0, 200}
};
uint8_t NUM_PALETTE = (sizeof palette) / (sizeof (struct palette));

uint16_t briLevels[] = {4, 16, 64, 256};
uint8_t NUM_BRILEVELS = (sizeof briLevels) / (sizeof (uint8));

uint8_t  MAC_STA[]                = {0,0,0,0,0,0};

void initWiFi() {
  // Connect to WiFi network
  Serial.print("MAC[STA]");
  uint8_t *MAC  = WiFi.macAddress(MAC_STA);                   //get MAC address of STA interface
  for (uint8_t i = 0; i < sizeof(MAC)+2; ++i){
    Serial.print(":");
    Serial.print(MAC[i],HEX);
    MAC_STA[i] = MAC[i];                                            //copy back to global variable
  }
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  
}

void initWiFi2() {
  if (!wiFiSetupDone) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("");
      Serial.println("WiFi connected");
      wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
      Serial.println("initializing server\r\n");
      initServer();
      Serial.println("ready for commands\r\n");
      initOta();
      wiFiSetupDone = true;
    }
  }
}

void setDynLed(uint16_t n, uint32_t c) {
  if (n < state.dynLevel) {
    setLed(n, c);
  }
  else {
    setLed(n, 0);
  }
}

void setLedsFixed(uint32_t c) {
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    setDynLed(i, c);
  }
}

void setLedsZylon() {
  const uint16 swipeTime = 5000;
  const uint16 swipeHalfWidth = NUM_LEDS / 10;
  const uint16 briOff = 64;
  const uint16 swipeTimeHalf = swipeTime / 2;
  uint16 swipeTPos = state.now % swipeTime;
  if (swipeTPos > swipeTimeHalf) {
    swipeTPos -= swipeTimeHalf;
    swipeTPos = swipeTimeHalf - swipeTPos;
  }
  const sint16 swipePos = swipeTPos * NUM_LEDS / swipeTimeHalf;
  uint32 cLow = ledColor(state.dynR * briOff / 256, state.dynG * briOff / 256, state.dynB * briOff / 256);
  uint32 cHigh = ledColor(state.dynR, state.dynG, state.dynB);
  for (uint16 i = 0; i < NUM_LEDS; i++) {
    if (i < swipePos - swipeHalfWidth || i > swipePos + swipeHalfWidth) {
      setDynLed(i, cLow);
    }
    else {
      setDynLed(i, cHigh);
    }
  }
}

void setLeds() {
  switch (settings.mode) {
    case 0:
      {
        setLedsFixed(ledColor(state.dynR, state.dynG, state.dynB));
      }
      break;
    case 1:
      {
        setLedsZylon();
      }
      break;
    default:
      {
        setLedsFixed(ledColor(0, 0, 0));
      }
      break;
  }
  sendLeds();
}

void loop() {
  static const int tickResolution = 1000;
  if (wiFiSetupDone) {
    handleServer();
  }
  else {
    initWiFi2();
  }
  updateState(NUM_LEDS);
  setLeds();
  handleOta();
  if (getLastLedChangeDelta() > 1000) {
    if (DEBUG_TIMING) {
      Serial.println("no color changes => sleeping");
    }
    wifi_set_sleep_type(LIGHT_SLEEP_T);
    delay(200);
  }
  if (DEBUG_TIMING && state.tick % tickResolution == 0) {
    static time_t lastTickTime = 0;
    Serial.print("tick time: ");
    Serial.print((state.now - lastTickTime) / (tickResolution * 1.0));
    Serial.println("ms");
    lastTickTime = state.now;
  }
  state.tick++;
  delay(10);
  yield();
}

void initState() {
  state.dynLevel = 256;
  state.now = millis();
  state.riseStart = state.now;
  state.riseStop = state.now + settings.rise;
  state.dynLevel = 10;
  setLeds();
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("initializing LED Strip\r\n");
  readSettings();
  initState();
  initLeds();
  Serial.println("initializing WIFI\r\n");
  initWiFi();
}


