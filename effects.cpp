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

void setLedsZylon() {
  const uint16_t swipeTime = 5000;
  const uint16_t swipeHalfWidth = NUM_LEDS / 10;
  const uint16_t briOff = 64;
  const uint16_t swipeTimeHalf = swipeTime / 2;
  uint16_t swipeTPos = state.now % swipeTime;
  if (swipeTPos > swipeTimeHalf) {
    swipeTPos -= swipeTimeHalf;
    swipeTPos = swipeTimeHalf - swipeTPos;
  }
  const int16_t swipePos = swipeTPos * NUM_LEDS / swipeTimeHalf;
  uint32_t cLow = ledColor(state.dynR * briOff / 256, state.dynG * briOff / 256, state.dynB * briOff / 256);
  uint32_t cHigh = ledColor(state.dynR, state.dynG, state.dynB);
  for (uint16 i = 0; i < NUM_LEDS; i++) {
    if (i < swipePos - swipeHalfWidth || i > swipePos + swipeHalfWidth) {
      setDynLed(i, cLow);
    }
    else {
      setDynLed(i, cHigh);
    }
  }
}


