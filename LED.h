#ifndef _LED_H_
#define _LED_H_

const uint16_t NUM_LEDS = 105;

extern uint32_t ledColor(uint8_t r, uint8_t g, uint8_t b);
extern uint32_t ledBriColor(uint16_t bri, uint8_t r, uint8_t g, uint8_t b);
extern void setLed(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
extern void setLed(uint16_t n, uint32_t c);
extern bool sendLeds();
extern time_t getLastLedChangeDelta();

extern void initLeds();
#endif

