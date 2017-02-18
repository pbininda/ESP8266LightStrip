#include <Arduino.h>
#include "LED.h"
#include "state.h"
#include "effects.h"

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

uint16_t cyclePos() {
  uint32_t cycle = state.now % settings.cycle;
  return cycle * 1024 / settings.cycle;
}

void setLedsZylon() {
  const uint16_t swipeHalfWidth = NUM_LEDS / 10;
  const uint16_t briOff = 64;
  uint16_t swipeTPos = cyclePos();
  if (swipeTPos > 512) {
    swipeTPos -= 512;
    swipeTPos = 512 - swipeTPos;
  }
  const int16_t swipePos = swipeTPos * NUM_LEDS / 512;
  uint32_t cLow = ledColor(state.dynR * briOff / 256, state.dynG * briOff / 256, state.dynB * briOff / 256);
  uint32_t cHigh = ledColor(state.dynR, state.dynG, state.dynB);
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    if (i < swipePos - swipeHalfWidth || i > swipePos + swipeHalfWidth) {
      setDynLed(i, cLow);
    }
    else {
      setDynLed(i, cHigh);
    }
  }
}


