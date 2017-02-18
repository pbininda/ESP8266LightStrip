#include <Arduino.h>
#include "state.h"

struct state state;
struct settings settings;

void updateState(uint8_t numLeds) {
  state.now = millis();
  if (settings.on) {
    if (state.now >= state.riseStart && state.now < state.riseStop) {
      state.dynLevel = (state.now - state.riseStart) * numLeds / (state.riseStop - state.riseStart);
    }
    else if (state.now >= state.riseStop) {
      state.dynLevel = numLeds;
    }
  }
  else {
    if (state.now >= state.fallStart && state.now < state.fallStop) {
      state.dynLevel = (state.fallStop - state.now) * numLeds / (state.fallStop - state.fallStart);
    }
    else if (state.now >= state.fallStop) {
      state.dynLevel = 0;
    }
  }
  state.dynFactor = settings.bri;
  state.dynR = state.dynFactor * settings.r / 256;
  state.dynG = state.dynFactor * settings.g / 256;
  state.dynB = state.dynFactor * settings.b / 256;
}

