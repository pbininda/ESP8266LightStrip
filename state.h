#ifndef _STATE_H_
#define _STATE_H_

extern struct settings {
  uint8_t on;
  uint8_t mode;
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint16_t bri;
  long rise;
  long fall;
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


extern struct palette {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} palette[];
extern uint8_t NUM_PALETTE;

extern uint16_t briLevels[];
extern uint8_t NUM_BRILEVELS;

extern void initState();
extern void updateState(uint8_t numLeds);

#endif
