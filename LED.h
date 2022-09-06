#ifndef _LED_H_
#define _LED_H_

const uint16_t NUM_LEDS = 28;

extern uint32_t ledColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
extern uint32_t ledBriColor(uint16_t bri, uint8_t r, uint8_t g, uint8_t b);
extern void setLed(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
extern void setLedc(uint16_t n, uint32_t c);
extern bool sendLeds();
extern time_t getLastLedChangeDelta();

extern void initLeds();
#endif*+
