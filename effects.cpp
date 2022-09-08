#include <Arduino.h>
#include "LED.h"
#include "state.h"
#include "effects.h"

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t wheel(uint16_t bri, uint8_t wheelPos) {
  wheelPos = 255 - wheelPos;
  if(wheelPos < 85) {
    return ledBriColor(bri, 255 - wheelPos * 3, 0, wheelPos * 3);
  }
  if(wheelPos < 170) {
    wheelPos -= 85;
    return ledBriColor(bri, 0, wheelPos * 3, 255 - wheelPos * 3);
  }
  wheelPos -= 170;
  return ledBriColor(bri, wheelPos * 3, 255 - wheelPos * 3, 0);
}

enum modes {GRADUAL_ON, OUTSIDE_IN, INSIDE_OUT, LTR};

enum modes mode = OUTSIDE_IN;

void setDynLed(uint16_t n, uint32_t c) {
  if (mode == GRADUAL_ON) {
    uint8_t bri = (c >> 24);
    uint32_t briAdj = bri * (uint32_t) state.dynLevel / DYNRANGE;
    c = (briAdj << 24) | (c & 0xFFFFFF);
    setLedc(n, c);
  } else if (mode == OUTSIDE_IN || mode == INSIDE_OUT) {
    int32_t p = (int32_t) NUM_LEDS - n * 2 - 1;
    if (p < 0) p = -p;
    if (mode == OUTSIDE_IN) {
      p = NUM_LEDS - p;
    }
    if (p < state.dynLevel * ((uint32_t )NUM_LEDS) / DYNRANGE) {
      setLedc(n, c);
    }
    else {
      setLedc(n, 0);
    }
  } else {
    if (n < state.dynLevel * (uint32_t )NUM_LEDS / DYNRANGE) {
      setLedc(n, c);
    }
    else {
      setLedc(n, 0);
    }
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

void setLedsRainbowCycle() {
  uint32_t offset = cyclePos();
  for(uint16_t i=0; i < NUM_LEDS; i++) {
    setDynLed(i, wheel(settings.bri2, ((i + (offset * NUM_LEDS / 1024)) * 256 / NUM_LEDS) & 255));
  }
}

void setLedsZylon() {
  const uint16_t swipeHalfWidth = NUM_LEDS / 10;
  const uint16_t briOff = settings.bri2 / 4;
  const uint16_t briOn = settings.bri2;
  uint16_t swipeTPos = cyclePos();
  if (swipeTPos > 512) {
    swipeTPos -= 512;
    swipeTPos = 512 - swipeTPos;
  }
  const int16_t swipePos = swipeTPos * NUM_LEDS / 512;
  uint32_t cLow = ledColor(state.dynR, state.dynG, state.dynB, briOff);
  uint32_t cHigh = ledColor(state.dynR, state.dynG, state.dynB, briOn);
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    if (i < swipePos - swipeHalfWidth || i > swipePos + swipeHalfWidth) {
      setDynLed(i, cLow);
    }
    else {
      setDynLed(i, cHigh);
    }
  }
}
