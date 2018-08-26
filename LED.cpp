#define BUS
// GPIO3 UART RXD0
//#define BUS_METHOD NeoEsp8266Dma800KbpsMethod
// GPIO2 UART TXD1

#define LEDAPA

#ifdef LEDWS
#define BUS_METHOD NeoEsp8266Uart800KbpsMethod
#define FEATURE NeoGrbFeature
#endif

#ifdef LEDAPA
#define BUS_METHOD DotStarSpiMethod2
#define FEATURE DotStarBgrFeature
#endif 

#ifdef BUS
#include <NeoPixelBus.h>
#include "DotStarSpiMethod2.h"
#else
#include <Adafruit_NeoPixel.h>
#endif
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

#ifdef BUS
uint32_t ledColor(uint8_t r, uint8_t g, uint8_t b) {
  return (((r << 8) | g) << 8) | b;
}

uint32_t getLed(uint16_t n) {
  RgbColor co = strip.GetPixelColor(n);
  return ledColor(co.R, co.G, co.B);
}
#else
uint32_t ledColor(uint8_t r, uint8_t g, uint8_t b) {
  return Adafruit_NeoPixel::Color(r, g, b);
}

uint32_t getLed(uint16_t n) {
  return strip.getPixelColor(n);
}
#endif

uint32_t ledBriColor(uint16_t bri, uint8_t r, uint8_t g, uint8_t b) {
  uint32_t br = bri;
  return ledColor(r * br / 256, g * br / 256, b * br / 256);
}


void setLed(uint16_t n, uint32_t c) {
  uint32_t oldC = getLed(n);
  if (oldC != c) {
    lastLedChange = state.now;
    ledsChanged = true;
#ifdef BUS
    uint8_t r;
    uint8_t g;
    uint8_t b;
    b = c & 255;
    c >>= 8;
    g = c & 255;
    c >>= 8;
    r = c & 255;
    strip.SetPixelColor(n, RgbColor(r, g, b));
#else
    strip.setPixelColor(n, c);
#endif
  }
}

void setLed(uint16_t n, uint8_t r, uint8_t g, uint8_t b) {
  setLed(n, ledColor(r, g, b));
}

bool sendLeds() {
  if (ledsChanged & strip.canShow()) {
    if (DEBUG_LED) {
      Serial.print("LEDs: ");
      for (uint16_t i = 0; i < NUM_LED_DEBUG; i++) {
        Serial.print(getLed(i) % 256 , HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    setLed(NUM_LEDS, 0, 0, 0);
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
#ifdef BUS
  Serial.println("using NeoPixelBus");
#else
  Serial.println("using Adafruit NeoPixel");
#endif
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  strip.begin();
  sendLeds(); // Initialize all pixels to 'off'
}


