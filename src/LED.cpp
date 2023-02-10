#include "settings.h"
#include "LED.h"
#include "state.h"
#include "values.h"

static const bool DEBUG_LED = false;
static const bool DEBUG_LED_NO_OUT = false;
static const int NUM_LED_DEBUG = 4;
static const int LED_PIN = 21;
static const int CLOCK_PIN = 19;

Led::Led(uint8_t stripNo, const State &state) :
    stripSettings(STRIP_SETTINGS[stripNo]),
    stripNo(stripNo),
    state(state),
    leds(new InternalRgbw[stripSettings.NUM_LEDS + 1]),
    fastLeds(new CRGB[stripSettings.NUM_LEDS + 1])
{
}

void Led::setLed(uint16_t n, const InternalRgbw &rgbwColor) {
  setLedc(n, ledColor(rgbwColor));
}

uint32_t Led::getLed(uint16_t n) const {
    return ledColor(leds[n]);
}

InternalRgbw Led::getLedc(uint16_t n) const {
  return leds[n];
}

time_t Led::getLastLedChangeDelta() const {
  return state.now - lastLedChange;
}

uint32_t Led::ledColor(const InternalRgbw &rgbwColor) {
  uint32_t col = rgbwColor.white;
  col <<= BITS_PER_BYTE;
  col |= rgbwColor.red;
  col <<= BITS_PER_BYTE;
  col |= rgbwColor.green;
  col <<= BITS_PER_BYTE;
  col |= rgbwColor.blue;
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
    leds[n].red = red;
    leds[n].green = green;
    leds[n].blue = blue;
    leds[n].white = white;
  }
}

static const int LED_BRI_SCALE = 8;
static const InternalRgbw black = {0, 0, 0, 0};

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
    setLed(stripSettings.NUM_LEDS, black);
    for (int i = 0; i < stripSettings.NUM_LEDS + 1; i++) {
      fastLeds[i] = CRGB(leds[i].red, leds[i].green, leds[i].blue);
      fastLeds[i].nscale8(leds[i].white * LED_BRI_SCALE);
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
