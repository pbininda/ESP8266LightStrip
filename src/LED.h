#ifndef _LED_H_
#define _LED_H_

#include "settings.h"
#include "state.h"

typedef struct internal_rgbw {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t w;
} INTERNAL_RGBW;

class Led {
    public:
        Led(uint8_t stripNo, const State &state);

        uint32_t ledColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) const;
        void setLed(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
        void setLedc(uint16_t n, uint32_t c);
        uint32_t getLed(uint16_t n) const;
        INTERNAL_RGBW getLedc(uint16_t n) const;
        bool sendLeds();
        time_t getLastLedChangeDelta() const;

        void initLeds();
        const StripSettings &stripSettings;

    private:
        uint8_t stripNo;
        const State &state;


        INTERNAL_RGBW *leds;
        void *fastLeds;
        bool ledsChanged = false;
        time_t lastLedChange = 0;

};

#endif
