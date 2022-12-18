#ifndef _LED_H_
#define _LED_H_

extern uint32_t ledColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w);
extern void setLed(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
extern void setLedc(uint16_t n, uint32_t c);
extern uint32_t getLed(uint16_t n);
extern bool sendLeds();
extern time_t getLastLedChangeDelta();

extern void initLeds();
#endif

