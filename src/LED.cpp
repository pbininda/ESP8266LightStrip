#define FASTLED_USE_GLOBAL_BRIGHTNESS 1

#include <FastLED.h>
#include "LED.h"
#include "state.h"

const int DEBUG_LED = 0;
const int DEBUG_LED_NO_OUT = 0;
const int NUM_LED_DEBUG = 4;
const int LED_PIN = 21;
const int CLOCK_PIN = 19;
const int TEST_PIN = 23;
int toggle = 0;

typedef struct internal_rgbw {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t w;
} INTERNAL_RGBW;

INTERNAL_RGBW leds[NUM_LEDS + 1];

CRGBArray<NUM_LEDS + 1> fastLeds;

bool ledsChanged = false;
time_t lastLedChange = 0;

time_t getLastLedChangeDelta() {
  return state.now - lastLedChange;
}

uint32_t ledColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  uint32_t c = w;
  c <<= 8;
  c |= r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

uint32_t getLed(uint16_t n) {
    return ledColor(leds[n].r, leds[n].g, leds[n].b, leds[n].w);
}

void setLedc(uint16_t n, uint32_t c) {
  uint32_t oldC = getLed(n);
  if (oldC != c) {
    lastLedChange = state.now;
    ledsChanged = true;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t w;
    b = c & 255;
    c >>= 8;
    g = c & 255;
    c >>= 8;
    r = c & 255;
    c >>= 8;
    w = c & 255;
    leds[n].r = r;
    leds[n].g = g;
    leds[n].b = b;
    leds[n].w = w;
  }
}

void setLed(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  setLedc(n, ledColor(r, g, b, w));
}

bool sendLeds() {
  digitalWrite(TEST_PIN, toggle ? HIGH : LOW);
  toggle = !toggle;
  if (ledsChanged) {
    if (DEBUG_LED) {
      Serial.print("LEDs: ");
      for (uint16_t i = 0; i < NUM_LED_DEBUG; i++) {
        Serial.print(getLed(i), HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    setLed(NUM_LEDS, 0, 0, 0, 0);
    for (int i = 0; i < NUM_LEDS + 1; i++) {
      fastLeds[i] = CRGB(leds[i].r, leds[i].g, leds[i].b);
      fastLeds[i].nscale8(leds[i].w * 8);
    }
    
    FastLED.show();
    ledsChanged = false;
    return true;
  }
  else {
    if (DEBUG_LED && DEBUG_LED_NO_OUT) {
      Serial.println("No LED output");
    }
    return false;
  }
}

void initLeds() {
  Serial.println("FastLED");
  pinMode(TEST_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(CLOCK_PIN, HIGH);
  FastLED.addLeds<APA102, LED_PIN, CLOCK_PIN, BGR, DATA_RATE_MHZ(1)>(fastLeds, NUM_LEDS + 1);
  sendLeds(); // Initialize all pixels to 'off'

}
