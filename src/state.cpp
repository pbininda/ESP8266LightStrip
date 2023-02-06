#include <stdint.h>
#include <time.h>
#include <Arduino.h>
#include "state.h"

struct state strip_states[NUM_STRIPS];
struct settings strip_settings[NUM_STRIPS];

void updateState(const struct settings &settings, struct state &state, uint8_t strip, uint8_t numLeds) {
  state.now = millis();
  if (settings.on) {
    if (state.now >= state.riseStart && state.now < state.riseStop) {
      state.dynLevel = (state.now - state.riseStart) * DYNRANGE / (state.riseStop - state.riseStart);
    }
    else if (state.now >= state.riseStop) {
      state.dynLevel = DYNRANGE;
    }
  }
  else {
    if (state.now >= state.fallStart && state.now < state.fallStop) {
      state.dynLevel = (state.fallStop - state.now) * DYNRANGE / (state.fallStop - state.fallStart);
    }
    else if (state.now >= state.fallStop) {
      state.dynLevel = 0;
    }
  }
  state.dynFactor = settings.bri;
}

void initState(const struct settings &settings, struct state &state) {
  state.dynLevel = 0;
  state.now = millis();
  state.riseStart = state.now;
  state.riseStop = state.now + settings.rise;
}

