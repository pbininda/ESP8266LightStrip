#include <stdint.h>
#include <time.h>
#include "settings.h"
#include "LED.h"
#include "state.h"
#include "effects.h"
#include "values.h"

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
static uint32_t wheel(uint16_t bri, uint8_t wheelPos) {
  wheelPos = LOW_8_BITS - wheelPos;
  if(wheelPos < LOW_8_BITS / 3) {
    return Led::ledColor({static_cast<uint8_t>(LOW_8_BITS - wheelPos * 3), 0, static_cast<uint8_t>(wheelPos * 3), static_cast<uint8_t>(bri)});
  }
  if(wheelPos < LOW_8_BITS * 2 / 3) {
    wheelPos -= LOW_8_BITS / 3;
    return Led::ledColor({0, static_cast<uint8_t>(wheelPos * 3), static_cast<uint8_t>(LOW_8_BITS - wheelPos * 3), static_cast<uint8_t>(bri)});
  }
  wheelPos -= NUM_IN_BYTE * 2 / 3;
  return Led::ledColor({static_cast<uint8_t>(wheelPos * 3), static_cast<uint8_t>(LOW_8_BITS - wheelPos * 3), 0, static_cast<uint8_t>(bri)});
}

static uint32_t adjusted_brightness(uint32_t col, uint32_t partOfDynRange) {
    const uint8_t bri = (col >> MAX_BRI);
    const uint32_t briAdj = bri * partOfDynRange / DYNRANGE;
    return (briAdj << MAX_BRI) | (col & UINT16_MAX);
}

static void setDynLed(const State &state, const Settings &settings, Led &led, const StripSettings &stripSettings, uint16_t n, uint32_t col) {
  const uint8_t mode = settings.onoffmode;
  if (mode == GRADUAL) {
    led.setLedc(n, adjusted_brightness(col, state.dynLevel));
  } else if (mode == OUTSIDE_IN || mode == INSIDE_OUT || mode == OUTSIDE_IN_SOFT || mode == INSIDE_OUT_SOFT) {
    int32_t point = stripSettings.NUM_LEDS - n * 2 - 1;
    if (point < 0) {
      point = -point;
    }
    if (mode == OUTSIDE_IN || mode == OUTSIDE_IN_SOFT) {
      point = stripSettings.NUM_LEDS - point;
    }
    const auto mark = static_cast<int32_t>(state.dynLevel * static_cast<uint32_t>(stripSettings.NUM_LEDS) / DYNRANGE);
    if (point < mark) {
      if (mode == OUTSIDE_IN_SOFT || mode == INSIDE_OUT_SOFT) {
        led.setLedc(n, adjusted_brightness(col, state.dynLevel));
      } else {
        led.setLedc(n, col);
      }
    }
    else {
      led.setLedc(n, 0);
    }
  } else if (mode == LTR || mode == RTL || mode == LTR_SOFT || mode == RTL_SOFT) {
    int32_t point = n;
    if (mode == RTL) {
      point = stripSettings.NUM_LEDS - n - 1;
    }
    if (point < state.dynLevel * static_cast<uint32_t>(stripSettings.NUM_LEDS) / DYNRANGE) {
      if (mode == LTR_SOFT || mode == RTL_SOFT) {
        led.setLedc(n, adjusted_brightness(col, state.dynLevel));        
      } else {
        led.setLedc(n, col);
      }
    }
    else {
      led.setLedc(n, 0);
    }
  } else {
    if (state.dynLevel > 0) {
      led.setLedc(n, col);
    } else {
      led.setLedc(n, 0);
    }
  }
}

static uint16_t cyclePos(const State &state, const Settings &settings) {
  const uint32_t cycle = state.now % settings.cycle;
  return cycle * CYCLE_SCALE / settings.cycle;
}

void Effects::setLedsFixed() {
  for (uint16_t i = 0; i < stripSettings.NUM_LEDS; i++) {
    const Palette pal = dynGradColor(i);
    const uint32_t col = Led::ledColor({pal.red, pal.green, pal.blue, settings.bri2});
    setDynLed(state, settings, led, stripSettings, i, col);
  }
}

void Effects::setLedsRainbowCycle() {
  const uint32_t offset = cyclePos(state, settings);
  for(uint16_t i=0; i < stripSettings.NUM_LEDS; i++) {
    setDynLed(state, settings, led, stripSettings, i, wheel(settings.bri2, ((i + (offset * stripSettings.NUM_LEDS / CYCLE_SCALE)) * NUM_IN_BYTE / stripSettings.NUM_LEDS) & LOW_8_BITS));
  }
}

void Effects::setLedsZylon() {
  const uint16_t swipeHalfWidth = stripSettings.NUM_LEDS / 10;
  const uint16_t briOff = settings.bri2 / 4;
  const uint16_t briOn = settings.bri2;
  uint16_t swipeTPos = cyclePos(state, settings);
  if (swipeTPos > NUM_IN_BYTE * 2) {
    swipeTPos -= NUM_IN_BYTE * 2;
    swipeTPos = NUM_IN_BYTE * 2 - swipeTPos;
  }
  const auto swipePos = static_cast<int16_t>(swipeTPos * stripSettings.NUM_LEDS / (NUM_IN_BYTE * 2));
  for (uint16_t i = 0; i < stripSettings.NUM_LEDS; i++) {
    const Palette pal = dynGradColor(i);
    const uint32_t cLow = Led::ledColor({pal.red, pal.green, pal.blue, static_cast<uint8_t>(briOff)});
    const uint32_t cHigh = Led::ledColor({pal.red, pal.green, pal.blue, static_cast<uint8_t>(briOn)});
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
      red: static_cast<uint8_t>(state.dynFactor * settings.palette[settings.colidx].red / NUM_IN_BYTE), // cppcheck-suppress internalAstError
      green: static_cast<uint8_t>(state.dynFactor * settings.palette[settings.colidx].green / NUM_IN_BYTE),
      blue: static_cast<uint8_t>(state.dynFactor * settings.palette[settings.colidx].blue / NUM_IN_BYTE)
    };
  }
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
  const uint8_t gradNum1 = ledIdx * (settings.ngradient - 1) / stripSettings.NUM_LEDS;
  const uint8_t gradNum2 = (gradNum1 + 1);
  const uint16_t gradLen = stripSettings.NUM_LEDS / (settings.ngradient - 1);
  const uint16_t gradPos = ledIdx - gradLen * gradNum1;
  const uint32_t gradPos256 = gradPos * LOW_8_BITS / gradLen;
  const Palette pal1 = settings.palette[(settings.colidx + gradNum1) % NUM_PALETTE];
  const Palette pal2 = settings.palette[(settings.colidx + gradNum2) % NUM_PALETTE];
  uint32_t red = (pal1.red * (LOW_8_BITS - gradPos256) + pal2.red * gradPos256) / NUM_IN_BYTE;
  if (red > LOW_8_BITS) {
    red = LOW_8_BITS;
  }
  uint32_t green = (pal1.green * (LOW_8_BITS - gradPos256) + pal2.green * gradPos256) / NUM_IN_BYTE;
  if (green > LOW_8_BITS) {
    green = LOW_8_BITS;
  }
  uint32_t blue = (pal1.blue * (LOW_8_BITS - gradPos256) + pal2.blue * gradPos256) / NUM_IN_BYTE;
  if (blue > LOW_8_BITS) {
    blue = LOW_8_BITS;
  }
  return {
    red: static_cast<uint8_t>(state.dynFactor * red / NUM_IN_BYTE),
    green: static_cast<uint8_t>(state.dynFactor * green / NUM_IN_BYTE),
    blue: static_cast<uint8_t>(state.dynFactor * blue / NUM_IN_BYTE)
  };
}
