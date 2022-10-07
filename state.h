#ifndef _STATE_H_
#define _STATE_H_

enum onoffmodes {NONE, GRADUAL, OUTSIDE_IN, OUTSIDE_IN_SOFT, INSIDE_OUT, INSIDE_OUT_SOFT, LTR, LTR_SOFT, RTL, RTL_SOFT, ONOFFMODE_LAST};

const uint8_t NUM_PALETTE = 8;
struct palette {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

extern struct settings {
  uint8_t on;
  uint8_t mode;
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint16_t bri;
  uint32_t rise;
  uint32_t fall;
  uint32_t cycle;
  uint8_t bri2;
  uint8_t onoffmode;
  struct palette palette[NUM_PALETTE];
} settings;

extern struct state {
  time_t now;
  time_t riseStart;
  time_t riseStop;
  time_t fallStart;
  time_t fallStop;
  uint16_t dynLevel;
  uint32_t dynFactor;
  uint8_t dynR;
  uint8_t dynG;
  uint8_t dynB;
  uint16_t tick;
} state;


extern uint16_t briLevels[];
extern uint8_t NUM_BRILEVELS;

extern void initState();
extern void updateState(uint8_t numLeds);

#define DYNRANGE (256L * 128L)

#endif
