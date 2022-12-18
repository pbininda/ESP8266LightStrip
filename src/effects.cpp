#include <stdint.h>
#include <time.h>
#include "settings.h"
#include "LED.h"
#include "state.h"
#include "effects.h"

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
static uint32_t wheel(uint16_t bri, uint8_t wheelPos) {
  wheelPos = 255 - wheelPos;
  if(wheelPos < 85) {
    return ledColor(255 - wheelPos * 3, 0, wheelPos * 3, bri);
  }
  if(wheelPos < 170) {
    wheelPos -= 85;
    return ledColor(0, wheelPos * 3, 255 - wheelPos * 3, bri);
  }
  wheelPos -= 170;
  return ledColor(wheelPos * 3, 255 - wheelPos * 3, 0, bri);
}

static uint32_t adjusted_brightness(uint32_t c, uint32_t partOfDynRange) {
    uint8_t bri = (c >> 24);
    uint32_t briAdj = bri * partOfDynRange / DYNRANGE;
    return (briAdj << 24) | (c & 0xFFFFFF);
}

static void setDynLed(uint16_t n, uint32_t c) {
  uint8_t mode = settings.onoffmode;
  if (mode == GRADUAL) {
    setLedc(n, adjusted_brightness(c, state.dynLevel));
  } else if (mode == OUTSIDE_IN || mode == INSIDE_OUT || mode == OUTSIDE_IN_SOFT || mode == INSIDE_OUT_SOFT) {
    int32_t p = (int32_t) NUM_LEDS - n * 2 - 1;
    if (p < 0) p = -p;
    if (mode == OUTSIDE_IN || mode == OUTSIDE_IN_SOFT) {
      p = NUM_LEDS - p;
    }
    int32_t mark = state.dynLevel * ((uint32_t )NUM_LEDS) / DYNRANGE;
    if (p < mark) {
      if (mode == OUTSIDE_IN_SOFT || mode == INSIDE_OUT_SOFT) {
        setLedc(n, adjusted_brightness(c, state.dynLevel));
      } else {
        setLedc(n, c);
      }
    }
    else {
      setLedc(n, 0);
    }
  } else if (mode == LTR || mode == RTL || mode == LTR_SOFT || mode == RTL_SOFT) {
    int32_t p = n;
    if (mode == RTL) {
      p = NUM_LEDS - n - 1;
    }
    if (p < state.dynLevel * (uint32_t )NUM_LEDS / DYNRANGE) {
      if (mode == LTR_SOFT || mode == RTL_SOFT) {
        setLedc(n, adjusted_brightness(c, state.dynLevel));        
      } else {
        setLedc(n, c);
      }
    }
    else {
      setLedc(n, 0);
    }
  } else {
    if (state.dynLevel > 0) {
      setLedc(n, c);
    } else {
      setLedc(n, 0);
    }
  }
}

static uint16_t cyclePos() {
  uint32_t cycle = state.now % settings.cycle;
  return cycle * 1024 / settings.cycle;
}

void setLedsFixed(uint32_t c) {
  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    setDynLed(i, c);
  }
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
