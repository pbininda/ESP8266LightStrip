#pragma once

#include "settings.h"

enum onoffmodes {NONE, GRADUAL, OUTSIDE_IN, OUTSIDE_IN_SOFT, INSIDE_OUT, INSIDE_OUT_SOFT, LTR, LTR_SOFT, RTL, RTL_SOFT, ONOFFMODE_LAST};

const uint8_t NUM_PALETTE = 24;

class Palette {
  public:
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

class Settings {
  public:
    constexpr Settings(): on(0), mode(0), colidx(0), bri(0), rise(0), fall(0), cycle(0), bri2(0), onoffmode(0), ngradient(0), palette({}) {};
    Settings(const Settings &) = delete;
    uint8_t on;
    uint8_t mode;
    uint8_t colidx;
    uint16_t bri;
    uint32_t rise;
    uint32_t fall;
    uint32_t cycle;
    uint8_t bri2;
    uint8_t onoffmode;
    uint8_t ngradient;
    Palette palette[NUM_PALETTE];
};

extern Settings strip_settings[NUM_STRIPS];

class State {
  public:
    constexpr State(): now(0), riseStart(0), riseStop(0), fallStart(0), fallStop(0), dynLevel(0), dynFactor(0), tick(0) {};
    State(const State&) = delete;
    time_t now;
    time_t riseStart;
    time_t riseStop;
    time_t fallStart;
    time_t fallStop;
    uint16_t dynLevel;
    uint32_t dynFactor;
    uint16_t tick;
};

extern State strip_states[NUM_STRIPS];

extern void initState(const Settings &settings, State &state);
extern void updateState(const Settings &settings, State &state);

#define DYNRANGE ((long) (256L * 128L))

// #define DYNR(state, settings, ledIdx) (state.dynFactor * settings.palette[settings.colidx].r / 256)
// #define DYNG(state, settings, ledIdx) (state.dynFactor * settings.palette[settings.colidx].g / 256)
// #define DYNB(state, settings, ledIdx) (state.dynFactor * settings.palette[settings.colidx].b / 256)
