#include "settings.h"
#include "LED.h"
#include "state.h"

static const bool DEBUG_LED = false;
static const bool DEBUG_LED_NO_OUT = false;
static const int NUM_LED_DEBUG = 4;
static const int LED_PIN = 21;
static const int CLOCK_PIN = 19;

static const uint8_t BITS_PER_BYTE = 8U;
static const uint16_t LOW_8_BITS = 255;

Led::Led(uint8_t stripNo, const State &state) :
    stripSettings(STRIP_SETTINGS[stripNo]),
    stripNo(stripNo),
    state(state),
    leds(new INTERNAL_RGBW[stripSettings.NUM_LEDS + 1]),
    fastLeds(new CRGB[stripSettings.NUM_LEDS + 1])
{
}

void Led::setLed(uint16_t n, uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
  setLedc(n, ledColor(red, green, blue, white));
}

uint32_t Led::getLed(uint16_t n) const {
    return ledColor(leds[n].r, leds[n].g, leds[n].b, leds[n].w);
}

INTERNAL_RGBW Led::getLedc(uint16_t n) const {
  return leds[n];
}

time_t Led::getLastLedChangeDelta() const {
  return state.now - lastLedChange;
}

uint32_t Led::ledColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t white) {
  uint32_t col = white;
  col <<= BITS_PER_BYTE;
  col |= red;
  col <<= BITS_PER_BYTE;
  col |= green;
  col <<= BITS_PER_BYTE;
  col |= blue;
  return col;
}

void Led::setLedc(uint16_t n, uint32_t col) {
  const uint32_t oldC = getLed(n);
  if (oldC != col) {
    lastLedChange = state.now;
    ledsChanged = true;
    const uint8_t blue = col & LOW_8_BITS;
    col >>= BITS_PER_BYTE;
    const uint8_t green = col & LOW_8_BITS;
    col >>= BITS_PER_BYTE;
    const uint8_t red = col & LOW_8_BITS;
    col >>= BITS_PER_BYTE;
    const uint8_t white = col & LOW_8_BITS;
    leds[n].r = red;
    leds[n].g = green;
    leds[n].b = blue;
    leds[n].w = white;
  }
}

bool Led::sendLeds() {
  if (ledsChanged) {
    if (DEBUG_LED) {
      Serial.print("LEDs: ");
      for (uint16_t i = 0; i < NUM_LED_DEBUG; i++) {
        Serial.print(getLed(i), HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    setLed(stripSettings.NUM_LEDS, 0, 0, 0, 0);
    for (int i = 0; i < stripSettings.NUM_LEDS + 1; i++) {
      ((CRGB *)fastLeds)[i] = CRGB(leds[i].r, leds[i].g, leds[i].b);
      ((CRGB *)fastLeds)[i].nscale8(leds[i].w * 8);
    }
    FastLED.show();
    ledsChanged = false;
    return true;
  }
  if (DEBUG_LED && DEBUG_LED_NO_OUT) {
    Serial.println("No LED output");
  }
  return false;
}
