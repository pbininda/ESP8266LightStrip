#define BUS
#define LEDAPA

#define BRIGHTNESS (0xE0 | 0)

#ifdef LEDWS
#define BUS_METHOD NeoEsp8266Uart800KbpsMethod
#define FEATURE NeoGrbFeature
#endif

#ifdef LEDAPA
#define BUS_METHOD DotStarSpi1MhzMethod
#define FEATURE DotStarLbgrFeature
#endif 

#include <NeoPixelBus.h>
// #include "DotStarSpiMethod2.h"
#include "LED.h"
#include "state.h"

const int DEBUG_LED = 0;
const int DEBUG_LED_NO_OUT = 0;
const int NUM_LED_DEBUG = 32;
const int LED_PIN = 2;
#ifdef BUS
#ifdef LEDWS
NeoPixelBus<FEATURE, BUS_METHOD> strip(NUM_LEDS + 1, LED_PIN);
#endif
#ifdef LEDAPA
NeoPixelBus<FEATURE, BUS_METHOD> strip(NUM_LEDS + 1);
#endif
#define canShow CanShow
#define begin Begin
#define show Show
#else
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS + 1, LED_PIN, NEO_GRB + NEO_KHZ800);
#endif

bool ledsChanged = false;
time_t lastLedChange = 0;

time_t getLastLedChangeDelta() {
  return state.now - lastLedChange;
}

uint32_t ledColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
//  uint8_t w = 0xE0 | settings.bri2;
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
  RgbwColor co = strip.GetPixelColor(n);
//  RgbColor co = strip.GetPixelColor(n);
  return ledColor(co.R, co.G, co.B, co.W);
}

uint32_t ledBriColor(uint16_t bri, uint8_t r, uint8_t g, uint8_t b) {
  uint32_t br = bri;
  return ledColor(r, g, b, br);
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
    strip.SetPixelColor(n, RgbwColor(r, g, b, w));
    // strip.SetPixelColor(n, RgbColor(r, g, b));
  }
}

void setLed(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  setLedc(n, ledColor(r, g, b, w));
}

bool sendLeds() {
  if (ledsChanged && strip.canShow()) {
    if (DEBUG_LED) {
      Serial.print("LEDs: ");
      for (uint16_t i = 0; i < NUM_LED_DEBUG; i++) {
        Serial.print(getLed(i) % 256 , HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    setLed(NUM_LEDS, 0, 0, 0, 0);
    strip.show();
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
  Serial.println("using NeoPixelBus");
  SPI.setFrequency(1000000L);
  strip.begin();
  sendLeds(); // Initialize all pixels to 'off'
}
