#include <stdint.h>
#include <time.h>
#include <HardwareSerial.h>
#include "settings.h"
#include "WiFiServe.h"
#include "OTA.h"
#include "HTTP.h"
#include "LED.h"
#include "state.h"
#include "persistence.h"
#include "effects.h"

uint16_t briLevels[] = {4, 16, 64, 256};
uint8_t NUM_BRILEVELS = (sizeof briLevels) / (sizeof briLevels[0]);

static const bool DEBUG_TIMING = 1;

static Led *leds[NUM_STRIPS];
static Effects *effects[NUM_STRIPS];

static void initData() {
  for (uint8_t i = 0; i < NUM_STRIPS; i++) {
    leds[i] = new Led(i, strip_states[i]);
    effects[i] = new Effects(i, strip_settings[i], strip_states[i], *leds[i]);
  }
}

static void initState() {
  for (uint8_t i = 0; i < NUM_STRIPS; i++) {
    initState(strip_settings[i], strip_states[i]);
  }
}

static void initLeds() {
#ifdef BADOBEN
  leds[0]->initLeds<STRIP_SETTINGS[0].LED_PIN, STRIP_SETTINGS[0].CLOCK_PIN>();
  // leds[1]->initLeds<STRIP_SETTINGS[1].LED_PIN, STRIP_SETTINGS[1].CLOCK_PIN>();
  // leds[2]->initLeds<STRIP_SETTINGS[0].LED_PIN, STRIP_SETTINGS[2].CLOCK_PIN>();
#elif BETT1
  leds[0]->initLeds<STRIP_SETTINGS[0].LED_PIN, STRIP_SETTINGS[0].CLOCK_PIN>();
  leds[1]->initLeds<STRIP_SETTINGS[1].LED_PIN, STRIP_SETTINGS[1].CLOCK_PIN>();
  // leds[2]->initLeds<STRIP_SETTINGS[0].LED_PIN, STRIP_SETTINGS[2].CLOCK_PIN>();
#elif BETT2
  leds[0]->initLeds<STRIP_SETTINGS[0].LED_PIN, STRIP_SETTINGS[0].CLOCK_PIN>();
  leds[1]->initLeds<STRIP_SETTINGS[1].LED_PIN, STRIP_SETTINGS[1].CLOCK_PIN>();
  // leds[2]->initLeds<STRIP_SETTINGS[0].LED_PIN, STRIP_SETTINGS[2].CLOCK_PIN>();
#elif KUECHE1
  leds[0]->initLeds<STRIP_SETTINGS[0].LED_PIN, STRIP_SETTINGS[0].CLOCK_PIN>();
  // leds[1]->initLeds<STRIP_SETTINGS[1].LED_PIN, STRIP_SETTINGS[1].CLOCK_PIN>();
  // leds[2]->initLeds<STRIP_SETTINGS[0].LED_PIN, STRIP_SETTINGS[2].CLOCK_PIN>();
#elif ESPTEST
  leds[0]->initLeds<STRIP_SETTINGS[0].LED_PIN, STRIP_SETTINGS[0].CLOCK_PIN>();
  // leds[1]->initLeds<STRIP_SETTINGS[1].LED_PIN, STRIP_SETTINGS[1].CLOCK_PIN>();
  // leds[2]->initLeds<STRIP_SETTINGS[0].LED_PIN, STRIP_SETTINGS[2].CLOCK_PIN>();
#else
#error define a system config
#endif
}

static void setLeds() {
  for (uint8_t i = 0; i < NUM_STRIPS; i++) {
    Settings &settings = strip_settings[i];
    switch (settings.mode) {
      default:
      case 0:
        {
          effects[i]->setLedsFixed();
        }
        break;
      case 1:
        {
          effects[i]->setLedsZylon();
        }
        break;
      case 2:
        {
          effects[i]->setLedsRainbowCycle();
        }

        break;
    }
    leds[i]->sendLeds();
  }
}

void loop() {
  handleWiFi(leds, effects);
  handleServer();
  handleOta();
  bool change = false;
  for (uint8_t i = 0; i < NUM_STRIPS; i++) {
    Settings &settings = strip_settings[i];
    State &state = strip_states[i];
    updateState(settings, state, i, STRIP_SETTINGS[i].NUM_LEDS);
    setLeds();
    if (leds[i]->getLastLedChangeDelta() <= 1000) {
      change = true;
    }
    state.tick++;
  }

  if (!change) {
    if (DEBUG_TIMING) {
      Serial.print("~");
    }
    wiFiGoToSleep(200);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println(String("initializing LED Strip for ") + SYSTEM_NAME + "\r\n");
  initData();
  readSettings();
  initState();
  initLeds();
  setLeds();
  Serial.println("initializing WIFI\r\n");
  initWiFi();
  Serial.println(String("StackSize at setup: ") + uxTaskGetStackHighWaterMark(NULL));
}
