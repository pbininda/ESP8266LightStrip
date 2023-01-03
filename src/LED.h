#ifndef _LED_H_
#define _LED_H_

#define FASTLED_USE_GLOBAL_BRIGHTNESS 1
#include <FastLED.h>


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

        template<uint8_t LED_PIN, uint8_t CLOCK_PIN>
        void initLeds() {
            Serial.println("FastLED");
            pinMode(ledPin, OUTPUT);
            pinMode(clockPin, OUTPUT);
            digitalWrite(ledPin, HIGH);
            digitalWrite(clockPin, HIGH);
            FastLED.addLeds<APA102, LED_PIN, CLOCK_PIN, BGR, DATA_RATE_MHZ(1)>((CRGB *)fastLeds, stripSettings.NUM_LEDS + 1);
            sendLeds(); // Initialize all pixels to 'off'
        }
        const StripSettings &stripSettings;

    private:
        uint8_t stripNo;
        const State &state;


        INTERNAL_RGBW *leds;
        void *fastLeds;
        bool ledsChanged = false;
        time_t lastLedChange = 0;
        uint8_t ledPin;
        uint8_t clockPin;

};

#endif
