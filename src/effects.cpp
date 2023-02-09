#include <stdint.h>
#include <time.h>
#include "settings.h"
#include "LED.h"
#include "state.h"
#include "effects.h"

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
static uint32_t wheel(const Led &led, uint16_t bri, uint8_t wheelPos) {
  wheelPos = 255 - wheelPos;
  if(wheelPos < 85) {
    return led.ledColor(255 - wheelPos * 3, 0, wheelPos * 3, bri);
  }
  if(wheelPos < 170) {
    wheelPos -= 85;
    return led.ledColor(0, wheelPos * 3, 255 - wheelPos * 3, bri);
  }
  wheelPos -= 170;
  return led.ledColor(wheelPos * 3, 255 - wheelPos * 3, 0, bri);
}

static uint32_t adjusted_brightness(uint32_t c, uint32_t partOfDynRange) {
    uint8_t bri = (c >> 24);
    uint32_t briAdj = bri * partOfDynRange / DYNRANGE;
    return (briAdj << 24) | (c & 0xFFFFFF);
}

static void setDynLed(const State &state, const Settings &settings, Led &led, const StripSettings &stripSettings, uint16_t n, uint32_t c) {
  uint8_t mode = settings.onoffmode;
  if (mode == GRADUAL) {
    led.setLedc(n, adjusted_brightness(c, state.dynLevel));
  } else if (mode == OUTSIDE_IN || mode == INSIDE_OUT || mode == OUTSIDE_IN_SOFT || mode == INSIDE_OUT_SOFT) {
    int32_t p = (int32_t) stripSettings.NUM_LEDS - n * 2 - 1;
    if (p < 0) p = -p;
    if (mode == OUTSIDE_IN || mode == OUTSIDE_IN_SOFT) {
      p = stripSettings.NUM_LEDS - p;
    }
    int32_t mark = state.dynLevel * ((uint32_t ) stripSettings.NUM_LEDS) / DYNRANGE;
    if (p < mark) {
      if (mode == OUTSIDE_IN_SOFT || mode == INSIDE_OUT_SOFT) {
        led.setLedc(n, adjusted_brightness(c, state.dynLevel));
      } else {
        led.setLedc(n, c);
      }
    }
    else {
      led.setLedc(n, 0);
    }
  } else if (mode == LTR || mode == RTL || mode == LTR_SOFT || mode == RTL_SOFT) {
    int32_t p = n;
    if (mode == RTL) {
      p = stripSettings.NUM_LEDS - n - 1;
    }
    if (p < state.dynLevel * (uint32_t ) stripSettings.NUM_LEDS / DYNRANGE) {
      if (mode == LTR_SOFT || mode == RTL_SOFT) {
        led.setLedc(n, adjusted_brightness(c, state.dynLevel));        
      } else {
        led.setLedc(n, c);
      }
    }
    else {
      led.setLedc(n, 0);
    }
  } else {
    if (state.dynLevel > 0) {
      led.setLedc(n, c);
    } else {
      led.setLedc(n, 0);
    }
  }
}

static uint16_t cyclePos(const State &state, const Settings &settings) {
  uint32_t cycle = state.now % settings.cycle;
  return cycle * 1024 / settings.cycle;
}

void Effects::setLedsFixed() {
  for (uint16_t i = 0; i < stripSettings.NUM_LEDS; i++) {
    Palette pal = dynGradColor(i);
    uint32_t col = led.ledColor(pal.r, pal.g, pal.b, settings.bri2);
    setDynLed(state, settings, led, stripSettings, i, col);
  }
}

void Effects::setLedsRainbowCycle() {
  uint32_t offset = cyclePos(state, settings);
  for(uint16_t i=0; i < stripSettings.NUM_LEDS; i++) {
    setDynLed(state, settings, led, stripSettings, i, wheel(led, settings.bri2, ((i + (offset * stripSettings.NUM_LEDS / 1024)) * 256 / stripSettings.NUM_LEDS) & 255));
  }
}

void Effects::setLedsZylon() {
  const uint16_t swipeHalfWidth = stripSettings.NUM_LEDS / 10;
  const uint16_t briOff = settings.bri2 / 4;
  const uint16_t briOn = settings.bri2;
  uint16_t swipeTPos = cyclePos(state, settings);
  if (swipeTPos > 512) {
    swipeTPos -= 512;
    swipeTPos = 512 - swipeTPos;
  }
  const int16_t swipePos = swipeTPos * stripSettings.NUM_LEDS / 512;
  for (uint16_t i = 0; i < stripSettings.NUM_LEDS; i++) {
    Palette p = dynGradColor(i);
    uint32_t cLow = led.ledColor(p.r, p.g, p.b, briOff);
    uint32_t cHigh = led.ledColor(p.r, p.g, p.b, briOn);
    if (i < swipePos - swipeHalfWidth || i > swipePos + swipeHalfWidth) {
      setDynLed(state, settings, led, stripSettings, i, cLow);
    }
    else {
      setDynLed(state, settings, led, stripSettings, i, cHigh);
    }
  }
}


Palette Effects::dynGradColor(uint16_t ledIdx) const {
  if (settings.ngradient <= 1) {
    return {
      r: (uint8_t) (state.dynFactor * settings.palette[settings.colidx].r / 256),
      g: (uint8_t) (state.dynFactor * settings.palette[settings.colidx].g / 256),
      b: (uint8_t) (state.dynFactor * settings.palette[settings.colidx].b / 256)
    };
  } else {
    // example:
    // NUM_LEDS = 10
    // ngradient = 2
    // gradLen = 10 / 1 = 10
    // gradnum(0) = 0, gradnum(4) = 0, gradnum(5) = 0, gradnum(9) = 0
    // gradPos(0) = 0, gradPos(4) = 4, gradnum(5) = 5, gradpos(9) = 9
    // pos256(0)  = 
    // NUM_LEDS = 10
    // ngradient = 3
    // gradnum(0) = 0, gradnum(4) = 0, gradnum(5) = 1, gradnum(9) = 1
    uint8_t gradNum1 = ledIdx * (settings.ngradient - 1) / stripSettings.NUM_LEDS;
    uint8_t gradNum2 = (gradNum1 + 1);
    uint16_t gradLen = stripSettings.NUM_LEDS / (settings.ngradient - 1);
    uint16_t gradPos = ledIdx - gradLen * gradNum1;
    uint32_t gradPos256 = gradPos * 255 / gradLen;
    const Palette p1 = settings.palette[(settings.colidx + gradNum1) % NUM_PALETTE];
    const Palette p2 = settings.palette[(settings.colidx + gradNum2) % NUM_PALETTE];
    uint32_t r = (p1.r * (255 - gradPos256) + p2.r * gradPos256) / 256;
    if (r > 255) r = 255;
    uint32_t g = (p1.g * (255 - gradPos256) + p2.g * gradPos256) / 256;
    if (g > 255) g = 255;
    uint32_t b = (p1.b * (255 - gradPos256) + p2.b * gradPos256) / 256;
    if (b > 255) b = 255;
    return {
      r: (uint8_t) (state.dynFactor * r / 256),
      g: (uint8_t) (state.dynFactor * g / 256),
      b: (uint8_t) (state.dynFactor * b / 256)
    };
  }
}
