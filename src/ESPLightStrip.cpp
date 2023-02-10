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
#include "mqtt.h"

static const bool DEBUG_TIMING = true;

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
    State::initState(strip_settings[i], strip_states[i]);
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
  leds[1]->initLeds<STRIP_SETTINGS[1].LED_PIN, STRIP_SETTINGS[1].CLOCK_PIN>();
  // leds[2]->initLeds<STRIP_SETTINGS[0].LED_PIN, STRIP_SETTINGS[2].CLOCK_PIN>();
#else
#error define a system config
#endif
}

static void setLeds() {
  for (uint8_t i = 0; i < NUM_STRIPS; i++) {
    const Settings &settings = strip_settings[i];
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

static const uint16_t MinChangeDelta = 1000;
static const uint16_t SleepDelay = 200;

void loop() {
  handleWiFi(leds, effects);
  handleServer();
  handleOta();
  handleMqtt();
  bool change = false;
  for (uint8_t i = 0; i < NUM_STRIPS; i++) {
    const Settings &settings = strip_settings[i];
    State &state = strip_states[i];
    State::updateState(settings, state);
    setLeds();
    if (leds[i]->getLastLedChangeDelta() <= MinChangeDelta) {
      change = true;
    }
    state.tick++;
  }

  if (!change) {
    if (DEBUG_TIMING) {
      Serial.print("~");
    }
    wiFiGoToSleep(SleepDelay);
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
  initMqtt();
  Serial.println(String("StackSize at setup: ") + uxTaskGetStackHighWaterMark(NULL));
}
