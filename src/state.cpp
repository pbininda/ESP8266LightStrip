#include <stdint.h>
#include <time.h>
#include <Arduino.h>
#include "state.h"

State strip_states[NUM_STRIPS];
Settings strip_settings[NUM_STRIPS];

void State::updateState(const Settings &settings, State &state) { // cppcheck-suppress unusedFunction
  state.now = millis();
  if (settings.on != 0) {
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

void State::initState(const Settings &settings, State &state) { // cppcheck-suppress unusedFunction
  state.dynLevel = 0;
  state.now = millis();
  state.riseStart = state.now;
  state.riseStop = state.now + settings.rise;
}

